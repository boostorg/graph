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
#include <vector>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <boost/graph/visitors.hpp>

// Sample output
//
//  parent[0] = 0
//  parent[1] = 3
//  parent[2] = 0
//  parent[3] = 4
//  parent[4] = 0


int main(int argc, char* argv[])
{
  using namespace boost;
  typedef adjacency_list < vecS, vecS, undirectedS, 
       plugin<vertex_color, default_color_type,
         plugin<vertex_distance,int> >, plugin<edge_weight,int> > Graph;
  typedef std::pair<int,int> E;
  const int num_nodes = 5;
  E edges[] = { E(0,2), 
                E(1,1), E(1,3), E(1,4),
                E(2,1), E(2,3),
                E(3,4),
                E(4,0) };
  int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1};
  Graph G(num_nodes, edges, edges + sizeof(edges)/sizeof(E), weights);

  std::vector<Graph::vertex_descriptor> p(num_vertices(G));
  prim_minimum_spanning_tree
    (G, *(vertices(G).first), get(vertex_distance(), G),
     make_ucs_visitor(record_predecessors(&p[0], on_edge_relaxed())));

  for ( std::vector<Graph::vertex_descriptor>::iterator i = p.begin();
        i != p.end(); ++i)
    std::cout << "parent[" << i - p.begin() 
	      << "] = " << *i << std::endl;
  return 0;
}



