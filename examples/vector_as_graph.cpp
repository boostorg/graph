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
#include <algorithm>
#include <iostream>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/pending/indirect_cmp.hpp>

using namespace std;
using namespace boost;

//  Sample output
//  order of discovery: s r w v t x y u 
//  order of finish: s r w v t x y u 

int main()
{
#ifndef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  enum { r, s, t, u, v, w, x, y, N };
  char name[] = { 'r', 's', 't', 'u', 'v', 'w', 'x', 'y' };

  typedef std::vector< std::vector<int> > Graph;
  Graph g(N);

  g[s].push_back(r);

  g[s].push_back(r);
  g[r].push_back(v);
  g[s].push_back(w);
  g[w].push_back(t);
  g[w].push_back(x);
  g[t].push_back(x);
  g[u].push_back(t); 
  g[x].push_back(y);
  g[y].push_back(u);

  vector<default_color_type> color(N, white(default_color_type()));
  vector<int> discover(N), finish(N);
  typedef vector<int>::iterator iter_t;
  int time = 0;
  breadth_first_search
    (g, int(s), make_bfs_visitor(make_pair(stamp_times(discover.begin(), time,
						       on_discover_vertex()),
					   stamp_times(finish.begin(), time,
						       on_finish_vertex()))),
     color.begin());

  std::cout << "order of discovery: ";

  // Perform some STL magic to order the vertices according to their
  // discover time
  vector<Graph::size_type> discover_order(N);
  boost::integer_range<int> ir(0,N);
  copy(ir.begin(), ir.end(), discover_order.begin());
  sort(discover_order.begin(), discover_order.end(),
       indirect_cmp<iter_t, less<int> >(discover.begin()));
  
  for (int i = 0; i < N; ++i)
    std::cout << name[ discover_order[i] ] << " ";
  std::cout << std::endl;
  
  std::cout << "order of finish: ";

  // Order vertices according to their finish time  
  vector<Graph::size_type> finish_order(N);
  copy(ir.begin(), ir.end(), finish_order.begin());
  sort(finish_order.begin(), finish_order.end(),
       indirect_cmp<iter_t, less<int> >(finish.begin()));
  
  for (int j = 0; j < N; ++j)
    std::cout << name[ finish_order[j] ] << " ";
  std::cout << std::endl;
#else
  std::cout << "The vec_adj_list module requires partial specialization" << std::endl;
#endif
  return 0;
}
