//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


int main() {
  typedef adjacency_list<vecS, vecS, bidirectionalS> graph_raw_t;
  
  // Build graph_raw1
  int num_vertices1 = 8; graph_raw_t graph_raw1(num_vertices1);
  add_edge(0, 6, graph_raw1); add_edge(0, 7, graph_raw1);
  add_edge(1, 5, graph_raw1); add_edge(1, 7, graph_raw1);
  add_edge(2, 4, graph_raw1); add_edge(2, 5, graph_raw1); add_edge(2, 6, graph_raw1);
  add_edge(3, 4, graph_raw1);
  
  // Build graph_raw2
  int num_vertices2 = 9; graph_raw_t graph_raw2(num_vertices2);
  add_edge(0, 6, graph_raw2); add_edge(0, 8, graph_raw2);
  add_edge(1, 5, graph_raw2); add_edge(1, 7, graph_raw2);
  add_edge(2, 4, graph_raw2); add_edge(2, 7, graph_raw2); add_edge(2, 8, graph_raw2);
  add_edge(3, 4, graph_raw2); add_edge(3, 5, graph_raw2); add_edge(3, 6, graph_raw2);
  
  typedef compressed_sparse_row_graph<bidirectionalS> graph_csr_t;
  
  graph_csr_t graph1(graph_raw1);
  graph_csr_t graph2(graph_raw2);
  
  // true instructs callback to verify a map using
  // verify_vf2_sub_graph_iso
  vf2_print_callback<graph_csr_t, graph_csr_t> callback(graph1, graph2, true);
  
  bool ret = vf2_sub_graph_iso(graph1, graph2, callback);
  (void)ret;
  
  return 0;
}
