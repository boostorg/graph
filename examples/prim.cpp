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

// The graph from p. 508 of CLR.
//
// Sample output
//
//  parent[a] = x
//  parent[b] = a
//  parent[c] = f
//  parent[d] = c
//  parent[e] = d
//  parent[f] = g
//  parent[g] = h
//  parent[h] = a
//  parent[i] = c
//

int main(int , char* [])
{
  using namespace boost;
  typedef adjacency_list<vecS, vecS, undirectedS, 
       property<vertex_color_t, default_color_type,
         property<vertex_distance_t,int> >, property<edge_weight_t,int> > 
    Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::pair<int,int> E;
  const int num_nodes = 9;
  char name[] = "abcdefghix";
  enum { a, b, c, d, e, f, g, h, i, x }; 
  E edges[] = { E(a,b), E(a,h),
                E(b,h), E(b,c),
		E(c,d), E(c,f), E(c,i),
		E(d,e), E(d,f),
		E(e,f), 
		E(f,g),
		E(g,i), E(g,h),
		E(h,i) };
  int weights[] = { 4, 8,
		    11, 8,
		    7, 4, 2,
		    9, 14,
		    10,
		    2,
		    6, 1,
		    7 };
		    
  Graph G(num_nodes, edges, edges + sizeof(edges)/sizeof(E), weights);

  std::vector<Vertex> p(num_vertices(G), x);
  prim_minimum_spanning_tree
    (G, *(vertices(G).first), get(vertex_distance, G),
     make_ucs_visitor(record_predecessors(&p[0], on_edge_relaxed())));

  for ( std::vector<Vertex>::iterator vi = p.begin();
        vi != p.end(); ++vi)
    std::cout << "parent[" << name[vi - p.begin()]
              << "] = " << name[*vi] << std::endl;
  return 0;
}



