//=======================================================================
// Copyright 2001 University of Notre Dame.
// Author: Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================

#include <iostream>
#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

int main(int,char*[])
{
  using namespace boost;
  typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
  typedef subgraph< adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_index_t, int> > > Graph;

  const int N = 6;
  Graph G0(N);

  Graph G1(G0), G2(G0);

  add_vertex(2, G1);
  add_vertex(4, G1);
  add_vertex(5, G1);
  
  add_edge(0, 2, G1); // (0,2) is subgraph G1 local indices for (2,5).

  add_vertex(0, G2);
  add_vertex(1, G2);

  add_edge(0, 1, G0);
  add_edge(1, 2, G0);
  add_edge(1, 3, G0);
  add_edge(4, 1, G0);
  add_edge(4, 5, G0);
  add_edge(5, 3, G0);
  
  std::cout << "G0:" << std::endl;
  print_graph(G0, get(vertex_index, G0));
  print_edges2(G0, get(vertex_index, G0), get(edge_index, G0));

  std::cout << "G1:" << std::endl;
  print_graph(G1, get(vertex_index, G1));
  print_edges2(G1, get(vertex_index, G1), get(edge_index, G1));

  std::cout << "G2:" << std::endl;
  print_graph(G2, get(vertex_index, G2));
  print_edges2(G2, get(vertex_index, G2), get(edge_index, G2));
  
  return 0;
}
