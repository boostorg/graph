//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <vector>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include "range_pair.hpp"

int
main()
{
  using namespace boost;
  GraphvizGraph g_dot;
  read_graphviz("figs/telephone-network.dot", g_dot);

  using Graph = adjacency_list<vecS, vecS, undirectedS, no_property,
    property<edge_weight_t, int>>;
  Graph g(num_vertices(g_dot));
  auto edge_attr_map = get(edge_attribute, g_dot);
  for (const auto& edge : make_range_pair(edges(g_dot))) {
    auto weight = std::stoi(edge_attr_map[edge]["label"]);
    property<edge_weight_t, int> edge_property(weight);
    add_edge(source(edge, g_dot), target(edge, g_dot), edge_property, g);
  }

  using Vertex = graph_traits < Graph >::vertex_descriptor;
  std::vector < Vertex > parent(num_vertices(g));
  auto weight = get(edge_weight, g);
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  auto indexmap = get(vertex_index, g);  
  std::vector<std::size_t> distance(num_vertices(g));
  prim_minimum_spanning_tree(g, *vertices(g).first, &parent[0], &distance[0],
                             weight, indexmap, default_dijkstra_visitor());
#else
  prim_minimum_spanning_tree(g, &parent[0]);
#endif

  int total_weight = 0;
  for (std::size_t v = 0; v < num_vertices(g); ++v)
    if (parent[v] != v)
      total_weight += get(weight, edge(parent[v], v, g).first);
  std::cout << "total weight: " << total_weight << std::endl;

  for (std::size_t  u = 0; u < num_vertices(g); ++u)
    if (parent[u] != u)
      edge_attr_map[edge(parent[u], u, g_dot).first]["color"] = "black";
  std::ofstream out("figs/telephone-mst-prim.dot");
  auto& graph_edge_attr_map = get_property(g_dot, graph_edge_attribute);
  graph_edge_attr_map["color"] = "gray";
  write_graphviz(out, g_dot);

  return EXIT_SUCCESS;
}
