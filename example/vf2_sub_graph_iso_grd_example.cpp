//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/array.hpp>
#include <boost/graph/grid_graph.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


int main() {
 
  typedef grid_graph<2> graph_type;
  // Build graph1
  // Define dimension lengths, a 2x2 in this case
  boost::array<std::size_t, 2> lengths1 = { { 2, 2 } };
  graph_type graph1(lengths1);
  
  // Build graph2
  // Define dimension lengths, a 2x3 in this case
  boost::array<std::size_t, 2> lengths2 = { { 2, 3 } };
  graph_type graph2(lengths2);
  
  // true instructs callback to verify a map using
  // verify_vf2_sub_graph_iso
  vf2_print_callback<graph_type, graph_type> callback(graph1, graph2, true);

  bool ret = vf2_sub_graph_iso(graph1, graph2, callback);
  (void)ret;
  
  return 0;
}
