// Copyright 2006 Trustees of Indiana University

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// A simple example of using read_graphviz to load a GraphViz Dot textual
// graph into a BGL adjacency_list graph

// Author: Ronald Garcia


#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <string>
#include <sstream>

using namespace boost;
using namespace std;

int main() {
  // Vertex properties
  using vertex_p = property < vertex_name_t, std::string,
            property < vertex_color_t, float > >;  
  // Edge properties
  using edge_p = property < edge_weight_t, double >;
  // Graph properties
  using graph_p = property < graph_name_t, std::string >;
  // adjacency_list-based type
  using graph_t = adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p >;

  // Construct an empty graph and prepare the dynamic_property_maps.
  graph_t graph(0);
  dynamic_properties dp;

  auto name = get(vertex_name, graph);
  dp.property("node_id",name);

  auto mass = get(vertex_color, graph);
  dp.property("mass",mass);

  auto weight = get(edge_weight, graph);
  dp.property("weight",weight);

  // Use ref_property_map to turn a graph property into a property map
  boost::ref_property_map<graph_t*,std::string> 
    gname(get_property(graph,graph_name));
  dp.property("name",gname);

  // Sample graph as an std::istream;
  std::istringstream
    gvgraph("digraph { graph [name=\"graphname\"]  a  c e [mass = 6.66] }");

  bool status = read_graphviz(gvgraph,graph,dp,"node_id");

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
