// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/strong_components.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
int
strong_components
  (Graph& g,
   const vector_property_map<int, typename Graph::VertexIndexMap>* in_component)
{
  typedef vector_property_map<int, typename Graph::VertexIndexMap> ComponentMap;

  ComponentMap component = 
    in_component? *in_component : g.template get_vertex_map<int>("component");

  return boost::strong_components (g, component,
                                   vertex_index_map(g.get_vertex_index_map()));
}

void export_strong_components()
{
  using boost::python::arg;
  using boost::python::def;

  def("strong_components", &strong_components<Digraph>,
      (arg("graph"), 
       arg("component_map") = 
         (vector_property_map<int, Digraph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
