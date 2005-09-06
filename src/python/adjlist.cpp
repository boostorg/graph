// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "basic_graph.hpp"
#include <boost/python.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <boost/graph/iteration_macros.hpp>

namespace boost { namespace graph { namespace python {

template<typename DirectedS>
void 
basic_graph<DirectedS>::read_adjlist(const std::string& filename, 
                                     const std::string& node_id)
{
  vector_property_map<std::string, VertexIndexMap> id_map 
    = get_vertex_map<std::string>(node_id);


  std::ifstream file_in(filename.c_str());
  std::string line;
  std::map<std::string, vertex_descriptor> name_to_vertex;

  while (std::getline(file_in, line)) {
    std::istringstream in(line);
    std::string src, tgt;
    if (in >> src >> tgt) {
      typename std::map<std::string, vertex_descriptor>::iterator pos;

      // Find or create the source vertex
      vertex_descriptor src_vert;
      pos = name_to_vertex.find(src);
      if (pos == name_to_vertex.end()) {
        src_vert = add_vertex();
        name_to_vertex[src] = src_vert;
        put(id_map, src_vert, src);
      } else {
        src_vert = pos->second;
      }

      // Find or create the target vertex
      vertex_descriptor tgt_vert;
      pos = name_to_vertex.find(tgt);
      if (pos == name_to_vertex.end()) {
        tgt_vert = add_vertex();
        name_to_vertex[tgt] = tgt_vert;
        put(id_map, tgt_vert, tgt);
      } else {
        tgt_vert = pos->second;
      }

      // Add the edge
      add_edge(src_vert, tgt_vert);
    }
  }
}

template<typename DirectedS>
void 
basic_graph<DirectedS>::write_adjlist(const std::string& filename, 
                                      const std::string& node_id)
{
  std::ofstream out(filename.c_str());
  VertexIndexMap index_map = get_vertex_index_map();

  bool have_id_map = has_vertex_map(node_id);
  vector_property_map<std::string, VertexIndexMap> id_map;
  if (have_id_map)
    id_map = get_vertex_map<std::string>(node_id);

  edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(); ei != ei_end; ++ei) {
    edge_descriptor e = *ei;
    if (have_id_map)
      out << get(id_map, source(e)) << ' ' 
          << get(id_map, target(e)) << std::endl;
    else
      out << get(index_map, source(e)) << ' ' 
          << get(index_map, target(e)) << std::endl;
  }
}

// Explicit instantiations
template 
  void 
   basic_graph<undirectedS>::read_adjlist(const std::string& filename,
                                           const std::string& node_id);
template 
  void 
   basic_graph<undirectedS>::write_adjlist(const std::string& filename,
                                            const std::string& node_id);

template 
  void 
   basic_graph<bidirectionalS>::read_adjlist(const std::string& filename,
                                              const std::string& node_id);
template 
  void 
   basic_graph<bidirectionalS>::write_adjlist(const std::string& filename,
                                               const std::string& node_id);

} } } // end namespace boost::graph::python
