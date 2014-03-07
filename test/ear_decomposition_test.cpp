// Copyright (c) 2014 Stefan Hammer, University of Vienna
// Copyright (c) 2014 Jakob Lykke Andersen, University of Southern Denmark
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/ear_decomposition.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <map>
#include <boost/graph/iteration_macros.hpp>
#include <boost/test/minimal.hpp>

using namespace boost;
using namespace std;

typedef adjacency_list< vecS, vecS, undirectedS,
    no_property,
    property<edge_color_t, default_color_type> > graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef graph_traits<graph_t>::edge_descriptor edge_t;

int test_main(int, char* []) {
  /** 0 - - 1 - - 2
   *  |     |     |
   *  7 - - 8 - - 3
   *  |           |
   *  6 - - 5 - - 4
   */
  
  const size_t N = 9;
  // graph
  graph_t g(N);
  // predecessor map
  typedef map<vertex_t,vertex_t> pred_t;
  pred_t pred;
  associative_property_map< pred_t > pm(pred);
  // add null_vertex to root in predecessor map
  pred[vertex(8, g)] = graph_traits<graph_t>::null_vertex();
  
  for(unsigned int i = 0; i < N-1; i++) { 
    // add tree-edges
    add_edge(i, i+1, g);
    // fill predecessor map
    pred[vertex(i, g)] = vertex(i+1, g);
  }
  // add cross-edges
  add_edge(0, 7, g);
  add_edge(1, 8, g);
  add_edge(8, 3, g);
  
  // ear map (this is what we want to fill!)
  typedef map<edge_t,int> ear_t;
  ear_t ear;
  associative_property_map< ear_t > em(ear);
  
  // call the algorithm
  int num = ear_decomposition(g, pm, em);
  
  BGL_FORALL_EDGES_T(e, g, graph_t) {
    cout << source(e, g) << "/" << target(e, g) << ": " << em[e] << endl;
  }
  
  ear_t ear_check;
  BGL_FORALL_EDGES_T(e, g, graph_t) { ear_check[e] = 3; }
  ear_check[edge(0, 1, g).first] = 1;
  ear_check[edge(0, 7, g).first] = 1;
  ear_check[edge(8, 3, g).first] = 2;
  
  BOOST_CHECK(ear == ear_check);
  BOOST_CHECK(num == 3);

  return 0;
}
