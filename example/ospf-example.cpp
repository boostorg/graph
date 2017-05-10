//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <fstream>              // for file I/O
#include <boost/graph/graphviz.hpp>     // for read/write_graphviz()
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/lexical_cast.hpp>
#include "range_pair.hpp"

namespace boost {
  enum graph_color_t { graph_color = 5556 };
  BOOST_INSTALL_PROPERTY(graph, color);
}

int
main()
{
  using namespace boost;
  using g_dot_type = 
    adjacency_list<vecS, vecS, directedS,
                   property<vertex_name_t, std::string>, 
                   property<edge_color_t, std::string,
                            property<edge_weight_t, int>>,
                   property<graph_color_t, std::string>>;
  g_dot_type g_dot;

  dynamic_properties dp(ignore_other_properties);
  dp.property("node_id", get(vertex_name, g_dot));
  dp.property("label", get(edge_weight, g_dot));
  dp.property("color", get(edge_color, g_dot));
  dp.property("color", ref_property_map<g_dot_type*, std::string>(get_property(g_dot, graph_color)));
  {
    std::ifstream infile("figs/ospf-graph.dot");
    read_graphviz(infile, g_dot, dp);
  }

  using Graph = adjacency_list<vecS, vecS, directedS, no_property,
    property<edge_weight_t, int>>;
  using vertex_descriptor = graph_traits<Graph>::vertex_descriptor;
  Graph g(num_vertices(g_dot));
  for (const auto& edge : make_range_pair(edges(g_dot))) {
    auto weight = get(edge_weight, g_dot, edge);
    property<edge_weight_t, int> edge_property(weight);
    add_edge(source(edge, g_dot), target(edge, g_dot), edge_property, g);
  }

  vertex_descriptor router_six;
  for (const auto& vertex : make_range_pair(vertices(g_dot)))
    if ("RT6" == get(vertex_name, g_dot, vertex)) {
      router_six = vertex;
      break;
    }

  std::vector<vertex_descriptor> parent(num_vertices(g));
  // All vertices start out as there own parent
  using size_type = graph_traits<Graph>::vertices_size_type;
  for (size_type p = 0; p < num_vertices(g); ++p)
    parent[p] = p;

#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  std::vector<int> distance(num_vertices(g));
  auto weightmap = get(edge_weight, g);
  auto indexmap = get(vertex_index, g);
  dijkstra_shortest_paths
    (g, router_six, &parent[0], &distance[0], weightmap,
     indexmap, std::less<int>(), closed_plus<int>(), 
     (std::numeric_limits<int>::max)(), 0, default_dijkstra_visitor());
#else
  dijkstra_shortest_paths(g, router_six, predecessor_map(&parent[0]));
#endif

  for (size_type i = 0; i < num_vertices(g); ++i)
    if (parent[i] != i) {
      auto e = edge(parent[i], i, g_dot).first;
      put(edge_color, g_dot, e, "black");
    }

  get_property(g_dot, graph_color) = "grey";
  {
    std::ofstream outfile("figs/ospf-sptree.dot");
    write_graphviz_dp(outfile, g_dot, dp);
  }

  std::ofstream rtable("routing-table.dat");
  rtable << "Dest    Next Hop    Total Cost" << std::endl;
  for (const auto& vertex : make_range_pair(vertices(g_dot)))
    if (parent[vertex] != vertex) {
      rtable << get(vertex_name, g_dot, vertex) << "    ";
      auto v = vertex;
      vertex_descriptor child;
      int path_cost = 0;
      auto weight_map = get(edge_weight, g);
      do {
        path_cost += get(weight_map, edge(parent[v], v, g).first);
        child = v;
        v = parent[v];
      } while (v != parent[v]);
      rtable << get(vertex_name, g_dot, child) << "     ";
      rtable << path_cost << std::endl;

    }

  return EXIT_SUCCESS;
}
