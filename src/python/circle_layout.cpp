// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/circle_layout.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "point2d.hpp"

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
circle_graph_layout
  (Graph& g,
   const vector_property_map<point2d, typename Graph::VertexIndexMap>* in_pos,
   double radius)
{
  typedef vector_property_map<point2d, typename Graph::VertexIndexMap> 
    PositionMap;

  PositionMap pos = 
    in_pos? *in_pos : g.template get_vertex_map<point2d>("position");

  circle_graph_layout(g, pos, radius);
}

void export_circle_graph_layout()
{
  using boost::python::arg;
  using boost::python::def;

  def("circle_graph_layout", 
      &circle_graph_layout<Graph>,
      (arg("graph"), 
       arg("position") = 
         (vector_property_map<point2d, Graph::VertexIndexMap>*)0,
       arg("radius") = 250.0));

  def("circle_graph_layout", 
      &circle_graph_layout<Digraph>,
      (arg("graph"), 
       arg("position") = 
         (vector_property_map<point2d, Digraph::VertexIndexMap>*)0,
       arg("radius") = 250.0));
}

} } } // end namespace boost::graph::python
