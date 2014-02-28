// Copyright (c) 2014 Stefan Hammer, University of Vienna
// Copyright (c) 2014 Jakob Lykke Andersen, University of Southern Denmark
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/open_ear_decomposition.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <map>
#include <vector>
#include <boost/graph/iteration_macros.hpp>

using namespace boost;
using namespace std;

typedef adjacency_list< vecS, vecS, undirectedS,
    no_property,
    property<edge_color_t, default_color_type> > graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef graph_traits<graph_t>::edge_descriptor edge_t;

int main(int, char**) {
  /** 0 - - 1 - - 2
   *  |     |     |
   *  7 - - 8 - - 3
   *  |           |
   *  6 - - 5 - - 4
   */
  
  const size_t N = 9;
  graph_t g(N);
  for(unsigned int i = 0; i < N; ++i) { add_edge(i, i+1, g); };
  add_edge(0, 7, g);
  add_edge(1, 8, g);
  add_edge(8, 3, g);
  
  typedef map<vertex_t,vertex_t> pred_t;
  pred_t pred;
  for (unsigned int i = 8; i >= 0; --i) { pred[i] = i-1; }
  associative_property_map< pred_t > pm(pred);
  
  typedef map<vertex_t,int> ear_t;
  ear_t ear;
  associative_property_map< ear_t > em(ear);
  
  open_ear_decomposition(g, pm, em);
  
  for (ear_t::iterator it = ear.begin(); it != ear.end(); it++) {
      cout << it->first << ", " << it->second << endl;
  }

  return EXIT_SUCCESS;
}
