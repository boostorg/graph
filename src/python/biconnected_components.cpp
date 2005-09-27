// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/biconnected_components.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template<typename Graph>
boost::python::list 
biconnected_components
  (Graph& g,
   const vector_property_map<int, typename Graph::EdgeIndexMap>* in_component)
{
  typedef vector_property_map<int, typename Graph::EdgeIndexMap> ComponentMap;

  ComponentMap component = 
    in_component? *in_component : g.template get_edge_map<int>("bicomponent");

  std::list<typename Graph::Vertex> art_points;
  boost::biconnected_components(g, component, std::back_inserter(art_points),
                                boost::vertex_index_map(g.get_vertex_index_map()));
  boost::python::list result;
  for (typename std::list<typename Graph::Vertex>::iterator i 
         = art_points.begin(); i != art_points.end(); ++i)
    result.append(*i);
  return result;
}

template<typename Graph>
boost::python::list
articulation_points(const Graph& g)
{
  std::list<typename Graph::Vertex> art_points;
  boost::python::list result;
  boost::articulation_points(g, std::back_inserter(art_points),
                             boost::vertex_index_map(g.get_vertex_index_map()));
  for (typename std::list<typename Graph::Vertex>::iterator i 
         = art_points.begin(); i != art_points.end(); ++i)
    result.append(*i);
  return result;
}

void export_biconnected_components()
{
  using boost::python::arg;
  using boost::python::def;

  def("biconnected_components", &biconnected_components<Graph>,
      (arg("graph"), 
       arg("component_map") = 
         (vector_property_map<int, Graph::EdgeIndexMap>*)0));
  def("articulation_points", &articulation_points<Graph>, (arg("graph")));

  def("biconnected_components", &biconnected_components<Digraph>,
      (arg("graph"), 
       arg("component_map") = 
         (vector_property_map<int, Digraph::EdgeIndexMap>*)0));
  def("articulation_points", &articulation_points<Digraph>, (arg("graph")));
}

} } } // end namespace boost::graph::python
