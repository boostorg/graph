// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/graph/betweenness_centrality.hpp>
#include <boost/graph/bc_clustering.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
brandes_betweenness_centrality
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>* in_vertex_centrality,
   const vector_property_map<double, typename Graph::EdgeIndexMap>* in_edge_centrality,
   const vector_property_map<double, typename Graph::EdgeIndexMap>* weight)
{
  typedef vector_property_map<double, typename Graph::VertexIndexMap> 
    VertexCentralityMap;

  typedef vector_property_map<double, typename Graph::EdgeIndexMap> 
    EdgeCentralityMap;

  VertexCentralityMap vertex_centrality = 
    in_vertex_centrality? *in_vertex_centrality 
    : g.template get_vertex_map<double>("centrality");

  EdgeCentralityMap edge_centrality = 
    in_edge_centrality? *in_edge_centrality 
    : g.template get_edge_map<double>("centrality");

  if (weight) {
    boost::brandes_betweenness_centrality
      (g, 
       weight_map(*weight).
       centrality_map(vertex_centrality).
       edge_centrality_map(edge_centrality).
       vertex_index_map(g.get_vertex_index_map()));
  } else {
    boost::brandes_betweenness_centrality
      (g, 
       centrality_map(vertex_centrality).
       edge_centrality_map(edge_centrality).
       vertex_index_map(g.get_vertex_index_map()));
  }
}

template<typename Graph>
void 
relative_betweenness_centrality
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>* in_centrality)
{ 
  typedef vector_property_map<double, typename Graph::VertexIndexMap> 
    CentralityMap;

  CentralityMap centrality = 
    in_centrality? *in_centrality 
    : g.template get_vertex_map<double>("centrality");

  relative_betweenness_centrality(g, centrality); 
}

template<typename Graph>
double
central_point_dominance
  (Graph& g, 
   const vector_property_map<double, typename Graph::VertexIndexMap>* in_centrality)
{ 
  typedef vector_property_map<double, typename Graph::VertexIndexMap> 
    CentralityMap;

  CentralityMap centrality = 
    in_centrality? *in_centrality 
    : g.template get_vertex_map<double>("centrality");

  return boost::central_point_dominance(g, centrality); 
}

struct bc_clustering_done_python
{
  explicit bc_clustering_done_python(boost::python::object done) 
    : done(done) { }

  template<typename Graph>
  bool 
  operator()(double max_centrality, 
             typename graph_traits<Graph>::edge_descriptor e,
             const Graph& g)
  {
    using boost::python::extract;
    return extract<bool>(done(max_centrality, e, ref(g)));
  }

private:
  boost::python::object done;
};

template<typename Graph>
void 
betweenness_centrality_clustering
  (Graph& g, boost::python::object done,
   const vector_property_map<double, typename Graph::EdgeIndexMap>* in_edge_centrality)
//   const vector_property_map<double, typename Graph::EdgeIndexMap>* weight)
{
  typedef vector_property_map<double, typename Graph::EdgeIndexMap> 
    EdgeCentralityMap;

  EdgeCentralityMap edge_centrality = 
    in_edge_centrality? *in_edge_centrality 
    : g.template get_edge_map<double>("centrality");

#if 0
  if (weight) {
    boost::betweenness_centrality_clustering
      (g, 
       weight_map(*weight).
       centrality_map(vertex_centrality).
       edge_centrality_map(edge_centrality).
       vertex_index_map(g.get_vertex_index_map()));
  } else {
#endif
    boost::betweenness_centrality_clustering(g, 
                                             bc_clustering_done_python(done),
                                             edge_centrality, 
                                             g.get_vertex_index_map());
    //  }
}

void export_betweenness_centrality()
{
  using boost::python::arg;
  using boost::python::def;

  // Graph
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality<Graph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Graph::VertexIndexMap>*)0,
       arg("edge_centrality_map") =
         (vector_property_map<double, Graph::EdgeIndexMap>*)0,
       arg("weight_map") = 
         (vector_property_map<double, Graph::EdgeIndexMap>*)0));
  def("relative_betweenness_centrality", 
      &relative_betweenness_centrality<Graph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Graph::VertexIndexMap>*)0));
  def("central_point_dominance",
      &central_point_dominance<Graph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Graph::VertexIndexMap>*)0));
  def("betweenness_centrality_clustering",
      &betweenness_centrality_clustering<Graph>,
      (arg("graph"),
       arg("done"),
       arg("edge_centrality_map") =
         (vector_property_map<double, Graph::EdgeIndexMap>*)0));

  // Digraph
  def("brandes_betweenness_centrality", 
      &brandes_betweenness_centrality<Digraph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Digraph::VertexIndexMap>*)0,
       arg("edge_centrality_map") =
         (vector_property_map<double, Digraph::EdgeIndexMap>*)0,
       arg("weight_map") = 
         (vector_property_map<double, Digraph::EdgeIndexMap>*)0));
  def("relative_betweenness_centrality", 
      &relative_betweenness_centrality<Digraph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Digraph::VertexIndexMap>*)0));
  def("central_point_dominance",
      &central_point_dominance<Digraph>,
      (arg("graph"),
       arg("vertex_centrality_map") = 
         (vector_property_map<double, Digraph::VertexIndexMap>*)0));
  def("betweenness_centrality_clustering",
      &betweenness_centrality_clustering<Digraph>,
      (arg("graph"),
       arg("done"),
       arg("edge_centrality_map") =
         (vector_property_map<double, Digraph::EdgeIndexMap>*)0));
}

} } } // end namespace boost::graph::python
