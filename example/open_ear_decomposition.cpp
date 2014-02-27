// Copyright (c) 2014 Stefan Hammer, University of Vienna
// Copyright (c) 2014 Jakob Lykke Andersen, University of Southern Denmark
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/open_ear_decomposition.hpp>
#include <boost/graph/grid_graph.hpp>
#include <boost/array.hpp>
#include <boost/random.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/graph/iteration_macros.hpp>

using namespace boost;
using namespace std;

typedef grid_graph<2> graph_type;
typedef graph_traits<graph_type> gt;

int main(int, char**) {

  boost::array<size_t, 2> sizes = {{ 5, 5 }};
  graph_type g(sizes);

  shared_array_property_map<gt::vertex_descriptor, property_map<graph_type, vertex_index_t>::const_type> pred(num_vertices(g), get(vertex_index, g));
  shared_array_property_map<int, property_map<graph_type, edge_index_t>::const_type> ear(num_edges(g), get(edge_index, g));

  boost::mt19937 gen;
  random_spanning_tree(g, gen, predecessor_map(pred));
  
  open_ear_decomposition(g, predecessor_map(pred), ear);

  return 0;
}
