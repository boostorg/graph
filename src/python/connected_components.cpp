// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/connected_components.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
int
connected_components
  (Graph& g,
   const vector_property_map<int, typename Graph::VertexIndexMap>* in_component,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<int, typename Graph::VertexIndexMap> ComponentMap;

  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ComponentMap component = 
    in_component? *in_component : g.template get_vertex_map<int>("component");

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  return boost::connected_components(g, component, color_map(color));
}

void export_connected_components()
{
  using boost::python::arg;
  using boost::python::def;

  def("connected_components", &connected_components<Graph>,
      (arg("graph"), 
       arg("component_map") = 
         (vector_property_map<int, Graph::VertexIndexMap>*)0,
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
