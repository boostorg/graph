// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "queue.hpp"

namespace boost { namespace graph { namespace python {

#define BGL_PYTHON_VISITOR dfs_visitor
#define BGL_PYTHON_EVENTS_HEADER "dfs_events.hpp"
#include "visitor.hpp"
#undef BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_VISITOR

template<typename Graph>
void 
depth_first_search
  (const Graph& g,
   typename Graph::Vertex s,
   const dfs_visitor<Graph>& visitor,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  bool has_default_visitor = 
    dynamic_cast<typename dfs_visitor<Graph>::default_arg const*>(&visitor);

  if (s == graph_traits<Graph>::null_vertex() && g.num_vertices() > 0)
    s = *g.vertices().first;


  if (has_default_visitor) {
    boost::depth_first_search(g, boost::dfs_visitor<>(), color, s);
  } else {
    boost::depth_first_search(g, typename dfs_visitor<Graph>::ref(visitor),
                              color, s);
  }
}
 
template<typename Graph>
void 
depth_first_visit
  (const Graph& g,
   typename Graph::Vertex s,
   const dfs_visitor<Graph>& visitor,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  bool has_default_visitor = 
    dynamic_cast<typename dfs_visitor<Graph>::default_arg const*>(&visitor);

  if (has_default_visitor) {
    boost::depth_first_visit(g, s, boost::dfs_visitor<>(), color);
  } else {
    boost::depth_first_visit(g, s, typename dfs_visitor<Graph>::ref(visitor),
                             color);
  }
}

template<typename Graph>
void 
undirected_dfs
  (const Graph& g,
   const dfs_visitor<Graph>& visitor,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color,
   const vector_property_map<default_color_type,
                             typename Graph::EdgeIndexMap>* in_edge_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;
  typedef vector_property_map<default_color_type, 
                              typename Graph::EdgeIndexMap> EdgeColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  EdgeColorMap edge_color = 
    in_edge_color? *in_edge_color 
    : EdgeColorMap(g.num_edges(), g.get_edge_index_map());

  bool has_default_visitor = 
    dynamic_cast<typename dfs_visitor<Graph>::default_arg const*>(&visitor);

  if (has_default_visitor) {
    boost::undirected_dfs(g, boost::dfs_visitor<>(), color, edge_color);
  } else {
    boost::undirected_dfs(g, typename dfs_visitor<Graph>::ref(visitor),
                          color, edge_color);
  }
}

void export_depth_first_search() 
{ 
  using boost::python::arg;
  using boost::python::def;

  def("depth_first_search", &depth_first_search<Graph>,
      (arg("graph"), 
       arg("root_vertex") = graph_traits<Graph>::null_vertex(),
       arg("visitor") = dfs_visitor<Graph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));

  def("depth_first_visit", &depth_first_visit<Graph>,
      (arg("graph"), 
       arg("root_vertex"),
       arg("visitor") = dfs_visitor<Graph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));

  def("undirected_dfs", &undirected_dfs<Graph>,
      (arg("graph"), 
       arg("visitor") = dfs_visitor<Graph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0,
       arg("edge_color_map") = 
         (vector_property_map<default_color_type, Graph::EdgeIndexMap>*)0));

  def("depth_first_search", &depth_first_search<Digraph>,
      (arg("graph"), 
       arg("root_vertex") = graph_traits<Graph>::null_vertex(),
       arg("visitor") = dfs_visitor<Digraph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Digraph::VertexIndexMap>*)0));

  def("depth_first_visit", &depth_first_visit<Digraph>,
      (arg("graph"), 
       arg("root_vertex"),
       arg("visitor") = dfs_visitor<Digraph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Digraph::VertexIndexMap>*)0));
}

template<typename Graph>
void export_depth_first_search_in_graph()
{
  dfs_visitor<Graph>::declare("DFSVisitor", "DefaultDFSVisitor");
}

template void export_depth_first_search_in_graph<Graph>();
template void export_depth_first_search_in_graph<Digraph>();

} } } // end namespace boost::graph::python
