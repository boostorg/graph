//=======================================================================
// Copyright 2009 Trustees of Indiana University.
// Authors: Michael Hansen, Andrew Lumsdaine
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/graph_archetypes.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/grid_graph.hpp>

#define DIMENSIONS 3
using namespace boost;

int main (int, char*[]) {

  typedef grid_graph<DIMENSIONS> Graph;
  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef graph_traits<Graph>::edge_descriptor Edge;

  function_requires<BidirectionalGraphConcept<Graph> >();
  function_requires<VertexListGraphConcept<Graph> >();
  function_requires<EdgeListGraphConcept<Graph> >();
  function_requires<IncidenceGraphConcept<Graph> >();
  function_requires<AdjacencyGraphConcept<Graph> >();
  function_requires<AdjacencyMatrixConcept<Graph> >();
  function_requires<ReadablePropertyGraphConcept<Graph, Vertex, vertex_index_t> >();
  function_requires<ReadablePropertyGraphConcept<Graph, Edge, edge_index_t> >();

  return (0);
}
