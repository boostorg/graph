//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <iostream>
#include <fstream>
#include "range_pair.hpp"

int
main()
{
  using namespace boost;
  using Graph = adjacency_list<vecS, vecS, undirectedS,
    no_property, property<edge_weight_t, int>>;
  using Edge = graph_traits<Graph>::edge_descriptor;
  using E = std::pair<int, int>;

  const int num_nodes = 5;
  E edge_array[] = { E(0, 2), E(1, 3), E(1, 4), E(2, 1), E(2, 3),
    E(3, 4), E(4, 0), E(4, 1)
  };
  int weights[] = { 1, 1, 2, 7, 3, 1, 1, 1 };
  std::size_t num_edges = sizeof(edge_array) / sizeof(E);
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  Graph g(num_nodes);
  auto = get(edge_weight, g);
  for (std::size_t j = 0; j < num_edges; ++j) {
    Edge e; bool inserted;
    std::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
    weightmap[e] = weights[j];
  }
#else
  Graph g(edge_array, edge_array + num_edges, weights, num_nodes);
#endif
  auto weight = get(edge_weight, g);
  std::vector<Edge> spanning_tree;

  kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

  std::cout << "Print the edges in the MST:" << std::endl;
  for (auto ei = spanning_tree.begin();
       ei != spanning_tree.end(); ++ei) {
    std::cout << source(*ei, g) << " <--> " << target(*ei, g)
      << " with weight of " << weight[*ei]
      << std::endl;
  }

  std::ofstream fout("figs/kruskal-eg.dot");
  fout << "graph A {\n"
    << " rankdir=LR\n"
    << " size=\"3,3\"\n"
    << " ratio=\"filled\"\n"
    << " edge[style=\"bold\"]\n" << " node[shape=\"circle\"]\n";
  for (const auto& edge : make_range_pair(edges(g))) {
    fout << source(edge, g) << " -- " << target(edge, g);
    if (std::find(spanning_tree.begin(), spanning_tree.end(), edge)
        != spanning_tree.end())
      fout << "[color=\"black\", label=\"" << get(edge_weight, g, edge)
           << "\"];\n";
    else
      fout << "[color=\"gray\", label=\"" << get(edge_weight, g, edge)
           << "\"];\n";
  }
  fout << "}\n";
  return EXIT_SUCCESS;
}
