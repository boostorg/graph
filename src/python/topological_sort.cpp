// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/topological_sort.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template<typename Graph>
boost::python::list
topological_sort
  (const Graph& g,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  std::list<typename Graph::Vertex> topo_order;
  boost::python::list result;
  boost::topological_sort(g, std::back_inserter(topo_order), color_map(color));
  for (typename std::list<typename Graph::Vertex>::iterator i 
         = topo_order.begin(); i != topo_order.end(); ++i)
    result.append(*i);
  return result;
}

void export_topological_sort()
{
  using boost::python::arg;
  using boost::python::def;

  def("topological_sort", &topological_sort<Graph>, 
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));
  def("topological_sort", &topological_sort<Digraph>, 
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<default_color_type, Digraph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
