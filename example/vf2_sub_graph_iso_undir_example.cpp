//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


int main() {
  typedef adjacency_list<vecS, vecS, bidirectionalS> graph_bidir_t;
  
  // Build graph1
  int num_vertices1 = 8; graph_bidir_t graph1(num_vertices1);
  add_edge(0, 6, graph1); add_edge(0, 7, graph1);
  add_edge(1, 5, graph1); add_edge(1, 7, graph1);
  add_edge(2, 4, graph1); add_edge(2, 5, graph1); add_edge(2, 6, graph1);
  add_edge(3, 4, graph1);
  // reversed edges
  add_edge(6, 0, graph1); add_edge(7, 0, graph1);
  add_edge(5, 1, graph1); add_edge(7, 1, graph1);
  add_edge(4, 2, graph1); add_edge(5, 2, graph1); add_edge(6, 2, graph1);
  add_edge(4, 3, graph1);
  
  add_edge(7, 7, graph1);
  add_edge(7, 7, graph1);


  // Build graph2
  int num_vertices2 = 9; graph_bidir_t graph2(num_vertices2);
  add_edge(0, 6, graph2); add_edge(0, 8, graph2);
  add_edge(1, 5, graph2); add_edge(1, 7, graph2);
  add_edge(2, 4, graph2); add_edge(2, 7, graph2); add_edge(2, 8, graph2);
  add_edge(3, 4, graph2); add_edge(3, 5, graph2); add_edge(3, 6, graph2);
  // reversed edges
  add_edge(6, 0, graph2); add_edge(8, 0, graph2);
  add_edge(5, 1, graph2); add_edge(7, 1, graph2);
  add_edge(4, 2, graph2); add_edge(7, 2, graph2); add_edge(8, 2, graph2);
  add_edge(4, 3, graph2); add_edge(5, 3, graph2); add_edge(6, 3, graph2);
  
  add_edge(5, 5, graph2);
  add_edge(5, 5, graph2);
  
  // Build graph3
  typedef adjacency_list<vecS, vecS, undirectedS> graph_undir_t;
  
  int num_vertices3 = 9; graph_undir_t graph3(num_vertices3);
  add_edge(0, 6, graph3); add_edge(0, 8, graph3);
  add_edge(1, 5, graph3); add_edge(1, 7, graph3);
  add_edge(2, 4, graph3); add_edge(2, 7, graph3); add_edge(2, 8, graph3);
  add_edge(3, 4, graph3); add_edge(3, 5, graph3); add_edge(3, 6, graph3);
  
  add_edge(5, 5, graph3);

  // true instructs callback to verify a map using
  // verify_vf2_sub_graph_iso
  vf2_print_callback<graph_bidir_t, graph_bidir_t> callback12(graph1, graph2, true);
  
  bool ret = vf2_sub_graph_iso(graph1, graph2, callback12);
  std::cout << std::endl;
  std::cout << std::endl;
  
  vf2_print_callback<graph_bidir_t, graph_undir_t> callback13(graph1, graph3, true);
  ret = vf2_sub_graph_iso(graph1, graph3, callback13);
  (void)ret;
  
  return 0;
}
