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
#include <vector>
#include <algorithm>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/graph_utility.hpp> // for iota
#include <boost/pending/indirect_cmp.hpp>
/*
   Depth-First Search

   This example records the "discover time" and "finish time" of each
   vertex during a depth-first search. This gives a little insight
   into the order in which DFS traverses a graph. It is instructive to
   compare this to the discover and finish time using breadth first
   search (in bfs_basics.cc). Note that the depth first search does
   not start at a particular source node, then visit all the other
   reachable vertices, as in BFS.  Instead it visits all vertices in
   the graph, possibly visiting several separate DFS trees. In the
   example below, one DFS tree is {u,x,v,y} and another is {w,z}.

   Here's the example graph from p. 479 of the CLR:

     u--+v   w
     |  +|  /|
     | / | / |
     +/  ++  |
     x+--y   z+_
              \_|

   Sample Output:

   order of discovery: u v y x w z 
   order of finish: x y v u z w 

   */

using namespace std;
using namespace boost;



int main(int argc, char* argv[]) 
{
  // Select the graph type we wish to use
  typedef adjacency_list<vecS, vecS, directedS> Graph;
  // Set up the vertex names
  enum { u, v, w, x, y, z, N };
  char name[] = { 'u', 'v', 'w', 'x', 'y', 'z' };
  int i;
  // Specify the edges in the graph
  typedef pair<int,int> E;
  E edge_array[] = { E(u,v), E(u,x), E(x,v), E(y,x),
		     E(v,y), E(w,y), E(w,z), E(z,z) };
  Graph G(N, edge_array, edge_array + sizeof(edge_array)/sizeof(E));

  // Some typedef's to save a little typing
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef boost::graph_traits<Graph>::vertices_size_type size_type;
  typedef std::vector<Vertex>::iterator Piter;
  // VC++ version of std::vector has no ::pointer, so
  // I use ::value_type* instead.
  typedef std::vector<size_type>::value_type* Iiter;

  // color, discover time, and finish time properties
  std::vector<default_color_type> color(num_vertices(G));
  std::vector<size_type> dtime(num_vertices(G));
  std::vector<size_type> ftime(num_vertices(G));
  int t = 0;
  depth_first_search
    (G, make_dfs_visitor(make_pair(stamp_times(&dtime[0], t, 
					       on_discover_vertex()),
				   stamp_times(&ftime[0], t, 
					       on_finish_vertex()))),
     &color[0]);

  // use std::sort to order the vertices by their discover time
  vector<size_type> discover_order(N);
  iota(discover_order.begin(), discover_order.end(), 0);
  std::sort(discover_order.begin(), discover_order.end(),
	    indirect_cmp<Iiter, std::less<size_type> >(&dtime[0]));

  cout << "order of discovery: ";
  for (i = 0; i < N; ++i)
    cout << name[ discover_order[i] ] << " ";

  vector<size_type> finish_order(N);
  iota(finish_order.begin(), finish_order.end(), 0);
  std::sort(finish_order.begin(), finish_order.end(),
	    indirect_cmp<Iiter, std::less<size_type> >(&ftime[0]));

  cout << endl << "order of finish: ";
  for (i = 0; i < N; ++i)
    cout << name[ finish_order[i] ] << " ";
  cout << endl;

  return 0;
}
