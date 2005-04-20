// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/isomorphism.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

template<typename Vertex>
struct py_vertex_invariant
{
  explicit py_vertex_invariant(boost::python::object invariant)
    : invariant(invariant) { }

  int operator()(const Vertex& v1, const Vertex& v2)
  { return boost::python::extract<int>(invariant(v1, v2)); }

private:
  boost::python::object invariant;
};

template<typename Graph>
bool
isomorphism
  (Graph& g1,
   Graph& g2,
   const vector_property_map<typename Graph::Vertex, 
                             typename Graph::VertexIndexMap>* in_iso,
   boost::python::object invariant)
{
  typedef typename Graph::Vertex Vertex;

  typedef vector_property_map<Vertex, typename Graph::VertexIndexMap> IsoMap;

  IsoMap iso = 
    in_iso? *in_iso 
    : IsoMap(num_vertices(g1), g1.get_vertex_index_map());

  if (invariant != boost::python::object()) 
    return boost::isomorphism
      (g1, g2, 
       isomorphism_map(iso).
       vertex_invariant(py_vertex_invariant<Vertex>(invariant)).
       vertex_index1_map(g1.get_vertex_index_map()).
       vertex_index2_map(g2.get_vertex_index_map()));
  else
    return boost::isomorphism
      (g1, g2, 
       isomorphism_map(iso).
       vertex_index1_map(g1.get_vertex_index_map()).
       vertex_index2_map(g2.get_vertex_index_map()));
}

void export_isomorphism()
{
  using boost::python::arg;
  using boost::python::def;
  using boost::python::object;

  def("isomorphism", &isomorphism<Graph>,
      (arg("g1"), arg("g2"),
       arg("isomorphism_map") = 
         (vector_property_map<Graph::Vertex, Graph::VertexIndexMap>*)0,
       arg("vertex_invariant") = object()));

  def("isomorphism", &isomorphism<Digraph>,
      (arg("g1"), arg("g2"),
       arg("isomorphism_map") = 
         (vector_property_map<Digraph::Vertex, Digraph::VertexIndexMap>*)0,
       arg("vertex_invariant") = object()));
}

} } } // end namespace boost::graph::python
