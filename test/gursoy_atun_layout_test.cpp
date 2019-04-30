// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Jeremiah Willcock
//           Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/gursoy_atun_layout.hpp>
#include "boost/graph/adjacency_list.hpp"
#include "boost/graph/random.hpp"
#include "boost/graph/graphviz.hpp"
#include "boost/random/mersenne_twister.hpp"
#include "boost/random/linear_congruential.hpp"
#include "boost/random/uniform_01.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <boost/core/lightweight_test.hpp>


using namespace boost;


template <class Property, class Vertex>
struct position_writer {
  const Property& property;

  position_writer(const Property& property): property(property) {}

  void operator()(std::ostream& os, const Vertex& v) const {
    os << "[pos=\"" << int(property[v][0]) << "," << int(property[v][1]) << "\"]";
  }
};

struct graph_writer {
  void operator()(std::ostream& os) const {
    os << "node [shape=point, width=\".01\", height=\".01\", fixedsize=\"true\"]"
       << std::endl;
  }
};

int main(int, char*[]) {
  // Generate a graph structured like a grid, cylinder, or torus; lay it out in
  // a square grid; and output it in dot format

  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                                boost::no_property,
                                boost::property<boost::edge_weight_t, double>
                                > graph_type;
  typedef boost::graph_traits<graph_type>::vertex_descriptor vertex_descriptor;

  graph_type graph;

  // Make grid, like Gursoy and Atun used
  std::map<int, std::map<int, vertex_descriptor> > verts;
  const int grid_size = 20;
  boost::minstd_rand edge_weight_gen;
  boost::uniform_01<boost::minstd_rand> random_edge_weight(edge_weight_gen);
  for (int i = 0; i < grid_size; ++i)
    for (int j = 0; j < grid_size; ++j)
      verts[i][j] = add_vertex(graph);
  for (int i = 0; i < grid_size; ++i) {
    for (int j = 0; j < grid_size; ++j) {
      if (i != 0)
        add_edge(verts[i][j], verts[i-1][j], random_edge_weight(), graph);
      if (j != 0)
        add_edge(verts[i][j], verts[i][j-1], random_edge_weight(), graph);
    }
  }

  typedef boost::property_map<graph_type, boost::vertex_index_t>::type
    VertexIndexMap;
  VertexIndexMap vertex_index = get(boost::vertex_index_t(), graph);

  typedef boost::heart_topology<> topology;
  topology space;

  typedef topology::point_type point;
  std::vector<point> position_vector(num_vertices(graph));
  typedef boost::iterator_property_map<std::vector<point>::iterator,
                                       VertexIndexMap, point, point&> Position;
  Position position(position_vector.begin(), vertex_index);

  boost::gursoy_atun_layout(graph, space, position);


  boost::gursoy_atun_layout(graph, space, position,
                            weight_map(get(boost::edge_weight, graph)));

  return boost::report_errors();
}
