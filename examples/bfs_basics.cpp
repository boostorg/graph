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
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp> // for iota
#include <boost/pending/indirect_cmp.hpp>

/* 
   This example records the "discover time" and "finish time"
   of each vertex during a breadth-first search. This
   gives a little insight into the order in which BFS
   traverses a graph. It is instructive to compare
   this to the discover and finish time using depth
   first search (in dfs_basics.cc).

   Here's the example graph from p. 471 of the CLR:

     r--s  t--u
     |  | /|  |  
     |  |/ |  |
     v  w--x--y

   Sample Output:

   order of discovery: s r w v t x u y 
   order of finish: s r w v t x u y 

*/

using namespace std;
using namespace boost;

int main(int, char*[]) 
{
  // Select the graph type we wish to use
  typedef adjacency_list<vecS, vecS, undirectedS> Graph;
  // Set up the vertex names
  enum { r, s, t, u, v, w, x, y, N };
  char name[] = { 'r', 's', 't', 'u', 'v', 'w', 'x', 'y' };
  int i;
  typedef pair<int,int> E;
  // Specify the edges in the graph
  E edge_array[] = { E(r,s), E(r,v), E(s,w),
                     E(w,r), E(w,t), E(w,x),
                     E(x,t), E(t,u), E(x,y),
                     E(u,y) };
  const int nedges = sizeof(edge_array) / sizeof(E);
  // Create the graph object
  Graph G(N, edge_array, edge_array + nedges);

  // Some typedef's to save a little typing
  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef graph_traits<Graph>::vertices_size_type size_type;
  
  typedef std::vector<Vertex>::iterator Piter;
  typedef std::vector<size_type>::iterator Iiter;

  // color property needed in breadth-first search
  std::vector<default_color_type> color(num_vertices(G));

  typedef boost::property_map<Graph, vertex_index_t>::type IndexMap;
  IndexMap vertex_id = get(vertex_index, G);

  // discover time property
  std::vector<size_type> dtime(num_vertices(G));

  // finish time property
  std::vector<size_type> ftime(num_vertices(G));

  typedef std::vector<size_type>::iterator RAIter;
  typedef random_access_iterator_property_map<RAIter, size_type, 
    size_type&, IndexMap> IterMap;

  IterMap discover(dtime.begin(), vertex_id);
  IterMap finish(ftime.begin(), vertex_id);

  // Call the 4 argument version of BFS. 
  // There is also a 3 argument version (assume color is in the graph)
  // and a 5 argument version (adds an argument for the queue).
  int time = 0;
  boost::breadth_first_search
    (G, vertex(s, G), make_bfs_visitor(
     std::make_pair(stamp_times(discover, time, on_discover_vertex()),
                    stamp_times(finish, time, on_finish_vertex()))),
     make_iterator_property_map(color.begin(), vertex_id, color[0]));

  cout << "order of discovery: ";

  // Perform some STL magic to order the vertices
  // according to their discover time
  std::vector<size_type> discover_order(N);
  iota(discover_order.begin(), discover_order.end(), 0);
  std::sort(discover_order.begin(), discover_order.end(),
            indirect_cmp<IterMap, std::less<size_type> >(discover));
  
  for (i = 0; i < N; ++i)
    cout << name[ discover_order[i] ] << " ";
  cout << endl;
  
  cout << "order of finish: ";
  
  vector<size_type> finish_order(N);
  iota(finish_order.begin(), finish_order.end(), 0);
  std::sort(finish_order.begin(), finish_order.end(),
            indirect_cmp<IterMap, std::less<size_type> >(finish));
  
  for (i = 0; i < N; ++i)
    cout << name[ finish_order[i] ] << " ";
  cout << endl;

  return 0;
}
