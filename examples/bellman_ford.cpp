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
#include <algorithm>
#include <iostream>
#include <vector>
#include <list>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/graph_utility.hpp>

//
// Example, page 533 CLR
//
//      u*===*v
//     *|\   **
//    / | \ / |
//   z  |  x  |
//    \ | / \ |
//     **/   *|
//      x----*y
//
// (didn't draw the y->z edge)
//
// Sample Output (distance and parent):
//
//  u: 2 v
//  v: 4 x
//  x: 7 z
//  y: -2 u
//  z: 0 z
//

int
main(int,char*)
{
  enum { u, v, x, y, z, N };
  char name[] = "uvxyz";

  typedef std::pair<int,int> E;
  E edges[] = { E(u,y), E(u,x), E(u,v),
                E(v,u),
                E(x,y), E(x,v),
                E(y,v), E(y,z),
                E(z,u), E(z,x) };
  
  int weight[] = { -4, 8, 5,
                   -2,
                   9, -3,
                   7, 2,
                   6, 7 };

  typedef boost::edge_list<E*, E, ptrdiff_t> Graph;
  Graph g(edges, edges + sizeof(edges) / sizeof(E));
    
  std::vector<int> distance(N, std::numeric_limits<short>::max());
  std::vector<int> parent(N, -1);
  
  distance[z] = 0;
  parent[z] = z;
  bool r = boost::bellman_ford_shortest_paths
    (g, int(N), weight, &distance[0], boost::make_bellman_visitor(
     boost::record_predecessors(&parent[0], boost::on_edge_relaxed())));

  if (r)  
    for (int i = 0; i < N; ++i)
      std::cout << name[i] << ": " << distance[i]
		<< " " << name[parent[i]] << std::endl;
  else
    std::cout << "negative cycle" << std::endl;
        
  return 0;
}
