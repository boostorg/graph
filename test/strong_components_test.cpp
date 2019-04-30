//=======================================================================
// Copyright 2014 Alexander Lauser.
// Authors: Alexander Lauser
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// This test is an adapted version of the MWE for Bug #10231 (showing
// incorrect root_map computation).

/* Output should be:
The example graph:
a --> b
b --> a c
c --> b

Vertex a is in component 0 and has root 0
Vertex b is in component 0 and has root 0
Vertex c is in component 0 and has root 0
*/
#include <boost/config.hpp>
#include <iostream>
#include <vector>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/core/lightweight_test.hpp>


int main(int, char*[])
{
  using namespace boost;

  adjacency_list<vecS, vecS, directedS> G;

  typedef graph_traits<adjacency_list<vecS, vecS, directedS> >::vertex_descriptor Vertex;
  Vertex a = add_vertex(G);
  Vertex b = add_vertex(G);
  Vertex c = add_vertex(G);

  add_edge(a, b, G);
  add_edge(b, a, G);

  add_edge(c, b, G);
  add_edge(b, c, G);

  std::vector<int> component(num_vertices(G)), discover_time(num_vertices(G));
  std::vector<default_color_type> color(num_vertices(G));
  std::vector<Vertex> root(num_vertices(G));
  strong_components(G, make_iterator_property_map(component.begin(), get(vertex_index, G)),
		    root_map(make_iterator_property_map(root.begin(), get(vertex_index, G))).
		    color_map(make_iterator_property_map(color.begin(), get(vertex_index, G))).
		    discover_time_map(make_iterator_property_map(discover_time.begin(), get(vertex_index, G))));

  for (std::vector<int>::size_type i = 0; i != component.size(); ++i) {
    BOOST_TEST_EQ(component[i], 0);
  }

  for (std::vector<int>::size_type i = 0; i != component.size(); ++i) {
    BOOST_TEST_EQ(root[i], 0);
  }

  return boost::report_errors();
}
