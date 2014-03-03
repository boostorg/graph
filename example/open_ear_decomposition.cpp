// Copyright (c) 2014 Stefan Hammer, University of Vienna
// Copyright (c) 2014 Jakob Lykke Andersen, University of Southern Denmark
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_spanning_tree.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/random.hpp>
#include <boost/graph/open_ear_decomposition.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <map>
#include <boost/graph/iteration_macros.hpp>

using namespace boost;

typedef adjacency_list< vecS, vecS, undirectedS,
    no_property,
    property<edge_color_t, default_color_type> > graph_t;
typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
typedef graph_traits<graph_t>::edge_descriptor edge_t;

graph_t get_graph () {
  graph_t g(8);
  for(unsigned int i = 0; i < 7; i++) { 
    add_edge(i, i+1, g);
  }
  add_edge(2, 7, g);
  add_edge(3, 6, g);
  add_edge(0, 4, g);
  
  return g;
}

int main(int argc, char* argv[])
{
  // get seed from arguments
  std::size_t seed = 1;
  if (argc > 1) seed = lexical_cast<std::size_t>(argv[1]);
  // generate a sample graph
  graph_t g = get_graph();
  // random generator
  mt19937 gen(seed);
  // predecessor map
  shared_array_property_map<vertex_t, property_map<graph_t, vertex_index_t>::const_type> pred(num_vertices(g), get(vertex_index, g));
  // get a random spanning tree
  random_spanning_tree(g, gen, predecessor_map(pred));
  // ear map (this is what we want to fill!)
  typedef std::map<edge_t,int> ear_t;
  ear_t ear;
  associative_property_map< ear_t > em(ear);
  
  // run the open ear decomposition
  open_ear_decomposition(g, pred, em);
  // print the EarMap
  BGL_FORALL_EDGES_T(e, g, graph_t) {
    std::cout << "(" << source(e, g) << "/" << target(e, g) << "): " << em[e] << std::endl;
  }
  
  return 0;
}