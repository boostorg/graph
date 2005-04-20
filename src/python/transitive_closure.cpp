// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/transitive_closure.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template<typename Graph>
std::auto_ptr<Graph>
transitive_closure
  (const Graph& g,
   const vector_property_map<typename Graph::Vertex, 
                             typename Graph::VertexIndexMap>* g_to_tc_map)
{
  std::auto_ptr<Graph> tc(new Graph);
  if (g_to_tc_map) 
    boost::transitive_closure(g, *tc, *g_to_tc_map, g.get_vertex_index_map());
  else
    boost::transitive_closure(g, *tc, 
                              vertex_index_map(g.get_vertex_index_map()));
  return tc;
}

void export_transitive_closure()
{
  using boost::python::arg;
  using boost::python::def;

  def("transitive_closure", &transitive_closure<Digraph>, 
      (arg("graph"),
       arg("orig_to_copy") = 
         (vector_property_map<Digraph::Vertex, Digraph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
