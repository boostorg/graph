//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <algorithm>
#include <utility>
#include <boost/graph/edmonds_karp_max_flow.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include "range_pair.hpp"

namespace boost
{
  template <typename Graph>
    std::pair < typename graph_traits<Graph>::vertex_descriptor,
    typename graph_traits<Graph >::degree_size_type>
    min_degree_vertex(Graph & g)
  {
    typename graph_traits<Graph>::vertex_descriptor p;
    using size_type = typename graph_traits<Graph>::degree_size_type;
    auto delta = (std::numeric_limits<size_type>::max)();
    for(const auto& vertex : make_range_pair(vertices(g)))
      if (degree(vertex, g) < delta)
      {
        delta = degree(vertex, g);
        p = vertex;
      }
    return std::make_pair(p, delta);
  }

  template <typename Graph, typename OutputIterator>
    void neighbors(const Graph & g,
                   typename graph_traits<Graph>::vertex_descriptor u,
                   OutputIterator result)
  {
    for (const auto& vertex : make_range_pair(adjacent_vertices(u, g)))
      *result++ = vertex;
  }
  template < typename Graph, typename VertexIterator,
    typename OutputIterator > void neighbors(const Graph & g,
                                             VertexIterator first,
                                             VertexIterator last,
                                             OutputIterator result)
  {
    for (; first != last; ++first)
      neighbors(g, *first, result);
  }

  template <typename VertexListGraph, typename OutputIterator>
  typename graph_traits<VertexListGraph>::degree_size_type
  edge_connectivity(VertexListGraph & g, OutputIterator disconnecting_set)
  {
    using vertex_descriptor = typename graph_traits <
      VertexListGraph >::vertex_descriptor;
    using degree_size_type = typename graph_traits <
      VertexListGraph >::degree_size_type;
    using Color = color_traits<default_color_type>;
    using edge_descriptor = typename adjacency_list_traits < vecS, vecS,
      directedS >::edge_descriptor;
    using FlowGraph = adjacency_list < vecS, vecS, directedS, no_property,
      property < edge_capacity_t, degree_size_type,
      property < edge_residual_capacity_t, degree_size_type,
      property<edge_reverse_t, edge_descriptor>>>>;

    vertex_descriptor u, v, k;
    bool inserted;
    degree_size_type alpha_star, alpha_S_k;
    std::set<vertex_descriptor> S, neighbor_S;
    std::vector<vertex_descriptor> S_star, nonneighbor_S;
    std::vector<default_color_type> color(num_vertices(g));
    std::vector<edge_descriptor> pred(num_vertices(g));

    FlowGraph flow_g(num_vertices(g));
    auto cap = get(edge_capacity, flow_g);
    auto res_cap = get(edge_residual_capacity, flow_g);
    auto rev_edge = get(edge_reverse, flow_g);

    for (const auto& edge : make_range_pair(edges(g))) {
      u = source(edge, g), v = target(edge, g);
      const auto [e1, inserted1] = add_edge(u, v, flow_g);
      cap[e1] = 1;
      const auto [e2, inserted2] = add_edge(v, u, flow_g);
      cap[e2] = 1;
      rev_edge[e1] = e2;
      rev_edge[e2] = e1;
    }

    const auto [p, delta] = min_degree_vertex(g);
    S_star.emplace_back(p);
    alpha_star = delta;
    S.insert(p);
    neighbor_S.insert(p);
    neighbors(g, S.begin(), S.end(),
              std::inserter(neighbor_S, neighbor_S.begin()));
    std::set_difference(vertices(g).first, vertices(g).second,
                        neighbor_S.begin(), neighbor_S.end(),
                        std::back_inserter(nonneighbor_S));

    while (!nonneighbor_S.empty()) {
      k = nonneighbor_S.front();
      alpha_S_k = edmonds_karp_max_flow
        (flow_g, p, k, cap, res_cap, rev_edge, &color[0], &pred[0]);
      if (alpha_S_k < alpha_star) {
        alpha_star = alpha_S_k;
        S_star.clear();
        for (const auto& vertex : make_range_pair(vertices(flow_g)))
          if (color[vertex] != Color::white())
            S_star.emplace_back(vertex);
      }
      S.insert(k);
      neighbor_S.insert(k);
      neighbors(g, k, std::inserter(neighbor_S, neighbor_S.begin()));
      nonneighbor_S.clear();
      std::set_difference(vertices(g).first, vertices(g).second,
                          neighbor_S.begin(), neighbor_S.end(),
                          std::back_inserter(nonneighbor_S));
    }

    std::vector<bool> in_S_star(num_vertices(g), false);
    for (const auto& vertex : S_star.begin())
      in_S_star[vertex] = true;
    degree_size_type c = 0;
    for (const auto& vertex : S_star.begin()) {
      for (const auto& edge : make_range_pair(out_edges(vertex, g)))
        if (!in_S_star[target(edge, g)]) {
          *disconnecting_set++ = edge;
          ++c;
        }
    }

    return c;
  }

}

int
main()
{
  using namespace boost;
  GraphvizGraph g;
  read_graphviz("figs/edge-connectivity.dot", g);

  using edge_descriptor = graph_traits<GraphvizGraph>::edge_descriptor;
  std::vector<edge_descriptor> disconnecting_set;
  auto c = edge_connectivity(g, std::back_inserter(disconnecting_set));

  std::cout << "The edge connectivity is " << c << "." << std::endl;

  auto attr_map = get(vertex_attribute, g);

  std::cout << "The disconnecting set is {";
  for (const auto& edge : disconnecting_set)
    std::cout << "(" << attr_map[source(edge, g)]["label"] << "," <<
      attr_map[target(edge, g)]["label"] << ") ";
  std::cout << "}." << std::endl;
  return EXIT_SUCCESS;
}
