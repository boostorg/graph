//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


int main() {
  typedef adjacency_list<vecS, vecS, bidirectionalS> graph_type;
  
  // Build graph1
  int num_vertices1 = 8; graph_type graph1(num_vertices1);
  add_edge(0, 6, graph1); add_edge(0, 7, graph1);
  add_edge(1, 5, graph1); add_edge(1, 7, graph1);
  add_edge(2, 4, graph1); add_edge(2, 5, graph1); add_edge(2, 6, graph1);
  add_edge(3, 4, graph1);

  // Build graph2
  int num_vertices2 = 9; graph_type graph2(num_vertices2);
  add_edge(0, 6, graph2); add_edge(0, 8, graph2);
  add_edge(1, 5, graph2); add_edge(1, 7, graph2);
  add_edge(2, 4, graph2); add_edge(2, 7, graph2); add_edge(2, 8, graph2);
  add_edge(3, 4, graph2); add_edge(3, 5, graph2); add_edge(3, 6, graph2);

  // true instructs callback to verify a map using
  // verify_vf2_sub_graph_iso
  vf2_print_callback<graph_type, graph_type> callback(graph1, graph2, true);

  bool ret = vf2_sub_graph_iso(graph1, graph2, callback);
  (void)ret;

  return 0;
}
