//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
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

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/transpose_graph.hpp>

/* Output:

  distances from start vertex:
  distance(0) = 0
  distance(1) = 6
  distance(2) = 1
  distance(3) = 4
  distance(4) = 5

  shortest paths tree
  0 --> 2 
  1 --> 
  2 --> 3 
  3 --> 4 
  4 --> 1 
*/

int 
main(int argc, char* argv[])
{
  using namespace boost;

  typedef property<edge_weight_t, int> weightp;
  typedef adjacency_list< listS, vecS, directedS, 
                      property<vertex_color_t,default_color_type>, weightp > Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;

  typedef std::pair<int,int> E;

  const int num_nodes = 5;
  E edges[] = { E(0,2), 
                E(1,1), E(1,3), E(1,4),
                E(2,1), E(2,3), 
                E(3,4),
                E(4,0), E(4,1) };
  int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1};

  std::cout << "constructing graph" << std::endl;
  Graph G(num_nodes, edges, edges + sizeof(edges)/sizeof(E), weights);
  std::cout << "done constructing" << std::endl;

  std::vector<Vertex> p(num_vertices(G));
  std::vector<int> d(num_vertices(G));

  Vertex s = *(vertices(G).first);
  
  dijkstra_shortest_paths(G, s, &d[0], 
    make_ucs_visitor(record_predecessors(&p[0], on_edge_relaxed())));

  std::cout << "distances from start vertex:" << std::endl;
  graph_traits<Graph>::vertex_iterator vi, vend;
  for(tie(vi,vend) = vertices(G); vi != vend; ++vi)
    std::cout << "distance(" << *vi << ") = " << d[*vi] << std::endl;
  std::cout << std::endl;

  std::cout << std::endl << "shortest paths tree" << std::endl;
  adjacency_list<> tree(num_nodes);
  tie(vi,vend) = vertices(G);
  for(++vi; vi != vend; ++vi)
    add_edge(p[*vi], *vi, tree);

  print_graph(tree);

  return 0;
}
