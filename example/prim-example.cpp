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
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

int
main()
{
  using namespace boost;
  typedef adjacency_list < vecS, vecS, undirectedS,
    property < vertex_distance_t, int, property < edge_weight_t, int >>>Graph;
  typedef std::pair < int, int >E;
  const int num_nodes = 5;
  E edges[] = { E(0, 2), E(1, 1), E(1, 3), E(1, 4), E(2, 1), E(2, 3),
    E(3, 4), E(4, 0)
  };
  int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1 };
  Graph g(num_nodes, edges, edges + sizeof(edges) / sizeof(E), weights);
  std::vector < graph_traits < Graph >::vertex_descriptor >
    p(num_vertices(G));

  prim_minimum_spanning_tree(g, &p[0]);

  std::vector < Graph::vertex_descriptor >::iterator i;
  for (i = p.begin(); i != p.end(); ++i)
    cout << "parent[" << i - p.begin()
      << "] = " << identity_property_map()[*i] << endl;
  else
  cout << "parent[" << i - p.begin() << "] = no parent" << endl;

  return EXIT_SUCCESS;
}
