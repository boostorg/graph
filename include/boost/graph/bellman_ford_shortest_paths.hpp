//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
//

#ifndef BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP
#define BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/relax.hpp>
#include <boost/graph/visitors.hpp>

namespace boost {

  template <class Visitor, class Graph>
  struct BellmanFordVisitor_concept {
    void constraints() {
      vis.examine_edge(e, g);
      vis.edge_relaxed(e, g);
      vis.edge_not_relaxed(e, g);
      vis.edge_minimized(e, g);
      vis.edge_non_minimized(e, g);
    }
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::edge_descriptor e;
  };

  template <class Visitors = null_visitor>
  class bellman_visitor {
  public:
    bellman_visitor(Visitors vis = null_visitor()) : m_vis(vis) { }

    template <class Edge, class Graph>
    void examine_edge(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_examine_edge());
    }
    template <class Edge, class Graph>
    void edge_relaxed(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_edge_relaxed());      
    }
    template <class Edge, class Graph>
    void edge_not_relaxed(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_edge_not_relaxed());
    }
    template <class Edge, class Graph>
    void edge_minimized(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_edge_minimized());
    }
    template <class Edge, class Graph>
    void edge_not_minimized(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_edge_not_minimized());
    }
  protected:
    Visitors m_vis;
  };
  template <class Visitors>
  bellman_visitor<Visitors>
  make_bellman_visitor(Visitors vis) {
    return bellman_visitor<Visitors>(vis);
  }

  // Variant (1)
  template <class EdgeListGraph, class Size, 
            class Weight, class Distance>
  bool bellman_ford_shortest_paths(EdgeListGraph& g, Size N, Weight w, 
                                   Distance d)
  {
    return bellman_ford_shortest_paths(g, N, w, d, bellman_visitor<>());
  }

  // Variant (2)
  template <class EdgeListGraph, class Size, class WeightPA, class DistancePA,
            class BellmanFordVisitor>
  bool bellman_ford_shortest_paths(EdgeListGraph& g, Size N, 
                                   WeightPA weight, DistancePA distance, 
				   BellmanFordVisitor v)
  {
    typedef typename graph_traits<EdgeListGraph>::edge_descriptor Edge;
    typedef typename graph_traits<EdgeListGraph>::vertex_descriptor
      Vertex;
    REQUIRE2(DistancePA, Vertex, ReadWritePropertyAccessor);
    REQUIRE2(WeightPA, Edge, ReadablePropertyAccessor);
    typedef typename property_traits<DistancePA>::value_type D_value;
    typedef typename property_traits<WeightPA>::value_type W_value;
    REQUIRE(D_value, LessThanComparable);
    REQUIRE3(D_value, D_value, W_value, _OP_PLUS);
    std::plus<D_value> combine;
    std::less<D_value> compare;
    return bellman_ford_shortest_paths(g, N, weight, distance, 
				       combine, compare, v);
  }

  // Variant (3)
  template <class EdgeListGraph, class Size, class WeightPA, class DistancePA,
            class BinaryFunction, class BinaryPredicate,
            class BellmanFordVisitor>
  bool bellman_ford_shortest_paths(EdgeListGraph& g, Size N, 
                                   WeightPA weight, DistancePA distance, 
				   BinaryFunction combine, 
				   BinaryPredicate compare,
				   BellmanFordVisitor v)
  {
    REQUIRE(EdgeListGraph, EdgeListGraph);
    typedef typename graph_traits<EdgeListGraph>::edge_descriptor Edge;
    typedef typename graph_traits<EdgeListGraph>::vertex_descriptor Vertex;
    REQUIRE2(DistancePA, Vertex, ReadWritePropertyAccessor);
    REQUIRE2(WeightPA, Edge, ReadablePropertyAccessor);
    typedef typename property_traits<DistancePA>::value_type D_value;
    typedef typename property_traits<WeightPA>::value_type W_value;

    typename graph_traits<EdgeListGraph>::edge_iterator i, end;
    
    for (Size k = 0; k < N; ++k)
      for (tie(i, end) = edges(g); i != end; ++i) {
	v.examine_edge(*i, g);
	if (relax(*i, g, weight, distance, combine, compare))
	  v.edge_relaxed(*i, g);
	else
	  v.edge_not_relaxed(*i, g);
      }

    for (tie(i, end) = edges(g); i != end; ++i)
      if (compare(combine(get(distance, source(*i, g)), 
			  get(weight, *i)),
		  get(distance, target(*i,g))))
      {
	v.edge_not_minimized(*i, g);
        return false;
      } else
	v.edge_minimized(*i, g);

    return true;
  }


} /* namespace boost */

#endif /* BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP */
