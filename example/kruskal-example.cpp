//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University,
// Bloomington, IN 47405.
//
// Permission to modify the code and to distribute the code is
// granted, provided the text of this NOTICE is retained, a notice if
// the code was modified is included with the above COPYRIGHT NOTICE
// and with the COPYRIGHT NOTICE in the LICENSE file, and that the
// LICENSE file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <fstream>

int
main()
{
  using namespace boost;
  typedef adjacency_list < vecS, vecS, undirectedS,
    no_property, property < edge_weight_t, int > > Graph;
  typedef graph_traits < Graph >::edge_descriptor Edge;
  typedef graph_traits < Graph >::vertex_descriptor Vertex;
  typedef std::pair < int, int >E;

  const int num_nodes = 5;
  E edge_array[] = { E(0, 2), E(1, 3), E(1, 4), E(2, 1), E(2, 3),
    E(3, 4), E(4, 0), E(4, 1)
  };
  int weights[] = { 1, 1, 2, 7, 3, 1, 1, 1 };
  int num_edges = sizeof(edge_array) / sizeof(E);
  Graph g(num_nodes, edge_array, edge_array + num_edges, weights);
  property_map < Graph, edge_weight_t >::type weight = get(edge_weight, g);
  std::vector < Edge > spanning_tree;

  kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));

  std::cout << "Print the edges in the MST:" << std::endl;
  for (std::vector < Edge >::iterator ei = spanning_tree.begin();
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
  graph_traits < Graph >::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    fout << source(*ei, g) << " -- " << target(*ei, g);
    if (std::find(spanning_tree.begin(), spanning_tree.end(), *ei)
        != spanning_tree.end())
      fout << "[color=\"black\"]\n";
    else
      fout << "[color=\"gray\"]\n";
  }
  fout << "}\n";
  return EXIT_SUCCESS;
}
