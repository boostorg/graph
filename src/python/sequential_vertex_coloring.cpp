// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/graph/sequential_vertex_coloring.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
int
sequential_vertex_coloring
  (Graph& g, 
   const vector_property_map<int, typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<int, typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color 
    : ColorMap(num_vertices(g), g.get_vertex_index_map());

  return boost::sequential_vertex_coloring(g, color);
}

void export_sequential_vertex_coloring()
{
  using boost::python::arg;
  using boost::python::def;

  def("sequential_vertex_coloring", 
      &sequential_vertex_coloring<Graph>,
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<int, Graph::VertexIndexMap>*)0));
  def("sequential_vertex_coloring", 
      &sequential_vertex_coloring<Digraph>,
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<int, Digraph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
