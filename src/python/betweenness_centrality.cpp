// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/graph/betweenness_centrality.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
brandes_betweenness_centrality_ve
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& vertex_centrality,
   const vector_property_map<double, typename Graph::EdgeIndexMap>& edge_centrality)
{
  brandes_betweenness_centrality
    (g, 
     centrality_map(vertex_centrality).
     edge_centrality_map(edge_centrality).
     vertex_index_map(g.get_vertex_index_map()));
}

template<typename Graph>
inline void 
brandes_betweenness_centrality_v
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& vertex_centrality)
{ 
  brandes_betweenness_centrality_ve(g, vertex_centrality,
                                    g.template get_edge_map<double>("centrality"));
}

template<typename Graph>
void 
brandes_betweenness_centrality_wve
  (Graph& g, 
   const vector_property_map<double, typename Graph::EdgeIndexMap>& weight,
   const vector_property_map<double, typename Graph::VertexIndexMap>& vertex_centrality,
   const vector_property_map<double, typename Graph::EdgeIndexMap>& edge_centrality)
{
  brandes_betweenness_centrality
    (g, 
     weight_map(weight).
     centrality_map(vertex_centrality).
     edge_centrality_map(edge_centrality).
     vertex_index_map(g.get_vertex_index_map()));
}

template<typename Graph>
inline void 
brandes_betweenness_centrality_wv
  (Graph& g, 
   const vector_property_map<double, typename Graph::EdgeIndexMap>& weight,
   const vector_property_map<double, typename Graph::VertexIndexMap>& vertex_centrality)
{
  brandes_betweenness_centrality_wve(g, weight, vertex_centrality,
                                     g.template get_edge_map<double>("centrality"));
}

template<typename Graph>
inline void 
brandes_betweenness_centrality_w
  (Graph& g, 
   const vector_property_map<double, typename Graph::EdgeIndexMap>& weight)
{
  brandes_betweenness_centrality_wv(g, weight, 
                                    g.template get_vertex_map<double>("centrality"));
}

template<typename Graph>
inline void 
brandes_betweenness_centrality(Graph& g)
{ 
  brandes_betweenness_centrality_v(g, g.template get_vertex_map<double>("centrality")); 
}

template<typename Graph>
void 
relative_betweenness_centrality
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& centrality)
{ relative_betweenness_centrality(g, centrality); }

template<typename Graph>
double
central_point_dominance
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>& centrality)
{ return boost::central_point_dominance(g, centrality); }

void export_betweenness_centrality()
{
  // Graph
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality<Graph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_v<Graph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_ve<Graph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_w<Graph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_wv<Graph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_wve<Graph>);
  def("relative_betweenness_centrality", 
      &relative_betweenness_centrality<Graph>);
  def("central_point_dominance", &central_point_dominance<Graph>);

  // Digraph
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality<Digraph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_v<Digraph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_ve<Digraph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_w<Digraph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_wv<Digraph>);
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality_wve<Digraph>);
  def("relative_betweenness_centrality", 
      &relative_betweenness_centrality<Digraph>);
  def("central_point_dominance", &central_point_dominance<Digraph>);
}

} } } // end namespace boost::graph::python
