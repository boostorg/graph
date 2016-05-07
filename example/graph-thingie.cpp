// Copyright 2008 Trustees of Indiana University

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// An example of using read_graphviz to load a GraphViz Dot textual
// graph into a BGL adjacency_list graph that has custom properties.

// Author: Ronald Garcia



#include <boost/graph/graphviz.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/foreach.hpp>
#include <string>
#include <sstream>
#include <cstdlib>

using namespace boost;


//
// Create a custom graph properties
//  (see the documentation for adjacency list)
struct graph_identifier_t { using kind = graph_property_tag; };
struct vertex_label_t { using kind = vertex_property_tag; };

int main() {

  // Vertex properties
  using vertex_p = property<vertex_name_t, std::string,
    property< vertex_label_t, std::string,
    property<vertex_root_t, int>>>;  
  // Edge properties
  using edge_p = property<edge_name_t, std::string>;
  // Graph properties
  using graph_p = property<graph_name_t, std::string,
    property<graph_identifier_t, std::string>>;
  // adjacency_list-based type
  using graph_t = adjacency_list<vecS, vecS, directedS,
    vertex_p, edge_p, graph_p>;

  // Construct an empty graph and prepare the dynamic_property_maps.
  graph_t graph(0);
  dynamic_properties dp;

  auto vname = get(vertex_name, graph);
  dp.property("node_id",vname);

  auto vlabel = get(vertex_label_t(), graph);
  dp.property("label",vlabel);

  auto root = get(vertex_root, graph);
  dp.property("root",root);

  auto elabel = get(edge_name, graph);
  dp.property("label",elabel);

  // Use ref_property_map to turn a graph property into a property map
  ref_property_map<graph_t*, std::string> 
    gname(get_property(graph,graph_name));
  dp.property("name",gname);

  // Use ref_property_map to turn a graph property into a property map
  ref_property_map<graph_t*, std::string> 
    gid(get_property(graph,graph_identifier_t()));
  dp.property("identifier",gid);
  // Sample graph as an istream;

const char* dot = 
"digraph \
{ \
  graph [name=\"GRAPH\", identifier=\"CX2A1Z\"] \
    \
    a [label=\"NODE_A\", root=\"1\"] \
    b [label=\"NODE_B\", root=\"0\"] \
 \
 a -> b [label=\"EDGE_1\"] \
 b -> c [label=\"EDGE_2\"] \
}";


  std::istringstream gvgraph(dot);

  bool status = read_graphviz(gvgraph,graph,dp,"node_id");
  if (!status) {
    std::cerr << "read_graphviz() failed." << std::endl;
    return -1;
  }

  std::cout << "graph " << get("name",dp,&graph) <<
      " (" << get("identifier",dp,&graph) << ")\n\n";

  BOOST_FOREACH( graph_t::vertex_descriptor v, vertices(graph) ) {
    std::cout << "vertex " << get("node_id",dp,v) <<
      " (" << get("label",dp,v) << ")\n";
  }

  return status ? EXIT_SUCCESS : EXIT_FAILURE;
}
