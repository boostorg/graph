// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/circle_layout.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "point2d.hpp"
#include <boost/graph/iteration_macros.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
kamada_kawai_spring_layout
  (Graph& g,
   const vector_property_map<point2d, typename Graph::VertexIndexMap>* in_pos,
   const vector_property_map<double, typename Graph::EdgeIndexMap>* in_weight,
   double side_length,
   boost::python::object done,
   double spring_constant,
   bool progressive)
{
  using boost::python::object;

  typedef vector_property_map<point2d, typename Graph::VertexIndexMap> 
    PositionMap;
  typedef vector_property_map<double, typename Graph::EdgeIndexMap> 
    WeightMap;

  PositionMap pos = 
    in_pos? *in_pos : g.template get_vertex_map<point2d>("position");

  WeightMap weight = 
    in_weight? *in_weight : WeightMap(num_edges(g), g.get_edge_index_map());

  // If we weren't give a weight map, assume unweighted edges
  if (!in_weight) BGL_FORALL_EDGES_T(e, g, Graph) put(weight, e, 1.0);

  if (!progressive) circle_graph_layout(g, pos, side_length/2);
  
  if (done != object()) {
    boost::kamada_kawai_spring_layout(g, pos, weight, 
                                      boost::side_length(side_length), done, 
                                      spring_constant,
                                      g.get_vertex_index_map());
  } else {
    boost::kamada_kawai_spring_layout(g, pos, weight, 
                                      boost::side_length(side_length), 
                                      layout_tolerance<double>(),
                                      spring_constant,
                                      g.get_vertex_index_map());
  }
}

void export_kamada_kawai_spring_layout()
{
  using boost::python::arg;
  using boost::python::def;
  using boost::python::object;

  def("kamada_kawai_spring_layout", 
      &kamada_kawai_spring_layout<Graph>,
      (arg("graph"), 
       arg("position") = 
         (vector_property_map<point2d, Graph::VertexIndexMap>*)0,
       arg("weight") = 
         (vector_property_map<double, Graph::EdgeIndexMap>*)0,
       arg("side_length") = 500.0,
       arg("done") = object(),
       arg("spring_constant") = 1.0,
       arg("progressive") = false));
}

} } } // end namespace boost::graph::python
