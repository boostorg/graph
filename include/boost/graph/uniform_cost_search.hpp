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
#ifndef BOOST_GRAPH_UNIFORM_COST_SEARCH_HPP
#define BOOST_GRAPH_UNIFORM_COST_SEARCH_HPP

#include <utility>
#include <vector>

#include <boost/config.hpp>
#include <boost/compose.hpp>
#include <boost/pending/mutable_queue.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/relax.hpp>
#include <boost/pending/indirect_cmp.hpp>

namespace boost {

  template <class Visitor, class Graph>
  struct UniformCostVisitorConcept {
    void constraints() {
      vis.initialize_vertex(u, g);
      vis.discover_vertex(u, g);
      vis.examine_vertex(u, g);
      vis.examine_edge(e, g);
      vis.edge_relaxed(e, g);
      vis.edge_not_relaxed(e, g);
      vis.finish_vertex(u, g);
    }
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::vertex_descriptor u;
    typename graph_traits<Graph>::edge_descriptor e;
  };

  template <class Visitors = null_visitor>
  class ucs_visitor : public bfs_visitor<Visitors> {
  public:
    ucs_visitor(Visitors vis = Visitors()) : bfs_visitor<Visitors>(vis) { }

    template <class Edge, class Graph>
    void edge_relaxed(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_edge_relaxed());      
    }
    template <class Edge, class Graph>
    void edge_not_relaxed(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_edge_not_relaxed());      
    }
  private:
    template <class Edge, class Graph>
    void tree_edge(Edge u, Graph& g) { }
  };
  template <class Visitors>
  ucs_visitor<Visitors>
  make_ucs_visitor(Visitors vis) {
    return ucs_visitor<Visitors>(vis);
  }

  // Variant (1)
  template <class VertexListGraph, class BinaryPredicate, 
            class BinaryFunction>
  inline void
  uniform_cost_search(VertexListGraph& g, 
              typename graph_traits<VertexListGraph>::vertex_descriptor s, 
              BinaryPredicate compare, BinaryFunction combine)
  {
    typedef typename graph_traits<VertexListGraph>::edge_descriptor Edge;
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor V;
    uniform_cost_search(g, s, get(vertex_distance, G), 
                        get(edge_weight, G), compare, combine);
  }

  // Variant (2)
  template <class VertexListGraph, class DistanceMap, class WeightMap,
            class BinaryPredicate, class BinaryFunction>
  inline void
  uniform_cost_search(VertexListGraph& g,
              typename graph_traits<VertexListGraph>::vertex_descriptor s, 
              DistanceMap d, WeightMap w,
              BinaryPredicate compare, BinaryFunction combine)
  {
    null_visitor null_vis;
    uniform_cost_search(g, s, d, w, 
                        get(vertex_color, g), get(vertex_index, g),
                        compare, combine, make_ucs_visitor(null_vis));
  }

  namespace detail {
    template <class UniformCostVisitor, class UpdatableQueue,
      class WeightMap, class DistanceMap, 
      class BinaryFunction, class BinaryPredicate>
    struct ucs_bfs_visitor {
      ucs_bfs_visitor(UniformCostVisitor vis, UpdatableQueue& Q,
                      WeightMap w, DistanceMap d, 
                      BinaryFunction combine, BinaryPredicate compare)
        : m_vis(vis), m_Q(Q), m_weight(w), m_distance(d), 
        m_combine(combine), m_compare(compare)  { }

      template <class Vertex, class Graph>
      void initialize_vertex(Vertex u, Graph& g) {
        m_vis.initialize_vertex(u, g);
      }
      template <class Vertex, class Graph>
      void discover_vertex(Vertex u, Graph& g) {
        m_vis.discover_vertex(u, g);
      }
      template <class Vertex, class Graph>
      void examine_vertex(Vertex u, Graph& g) {
        m_vis.examine_vertex(u, g);
      }
      template <class Edge, class Graph>
      void examine_edge(Edge e, Graph& g) { 
        m_vis.examine_edge(e, g);
      }
      template <class Edge, class Graph>
      void tree_edge(Edge e, Graph& g) {
        m_decreased = relax(e, g, m_weight, m_distance, 
                            m_combine, m_compare);
        if (m_decreased) {
          m_vis.edge_relaxed(e, g);
        } else
          m_vis.edge_not_relaxed(e, g);
      }
      template <class Edge, class Graph>
      void non_tree_edge(Edge, Graph&) { }

      template <class Edge, class Graph>
      void gray_target(Edge e, Graph& g) {
        m_decreased = relax(e, g, m_weight, m_distance, 
                            m_combine, m_compare);
        if (m_decreased) {
          m_Q.update(target(e, g));
          m_vis.edge_relaxed(e, g);
        } else
          m_vis.edge_not_relaxed(e, g);
      }
      template <class Edge, class Graph>
      void black_target(Edge, Graph&) { }

      template <class Vertex, class Graph>
      void finish_vertex(Vertex u, Graph& g) {
        m_vis.finish_vertex(u, g);
      }
      UniformCostVisitor m_vis;
      UpdatableQueue& m_Q;
      WeightMap m_weight;
      DistanceMap m_distance;
      BinaryFunction m_combine;
      BinaryPredicate m_compare;
      bool m_decreased;
    };
  } // namespace detail

  // Variant (3)
  template <class VertexListGraph,
            class DistanceMap, class WeightMap, class ColorMap, class IndexMap,
            class BinaryPredicate, class BinaryFunction,
            class UniformCostVisitor>
  void
  uniform_cost_search(VertexListGraph& g,
              typename graph_traits<VertexListGraph>::vertex_descriptor  s,
              DistanceMap d, WeightMap w, ColorMap color, IndexMap id, 
              BinaryPredicate compare, BinaryFunction combine,
              UniformCostVisitor vis)
  {
    function_requires<VertexListGraphConcept<VertexListGraph> >();
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
    typedef typename graph_traits<VertexListGraph>::edge_descriptor Edge;
    function_requires<ReadWritePropertyMapConcept<ColorMap, Vertex> >();
    function_requires<ReadWritePropertyMapConcept<DistanceMap, Vertex> >();
    function_requires<ReadablePropertyMapConcept<WeightMap, Edge> >();
    function_requires<ReadablePropertyMapConcept<IndexMap, Vertex> >();
    typedef typename property_traits<DistanceMap>::value_type D_value;
    typedef typename property_traits<WeightMap>::value_type W_value;
    typedef typename property_traits<IndexMap>::value_type ID_value;
    typedef typename property_traits<ColorMap>::value_type Color_value;
    function_requires<ColorValueConcept<Color_value> >();
    function_requires<BinaryPredicateConcept
      <BinaryPredicate, D_value, D_value> >();
    function_requires<BinaryFunctionConcept
      <BinaryFunction, D_value, D_value, W_value> >();
    function_requires<IntegerConcept<ID_value> >();
    function_requires<UniformCostVisitorConcept
      <UniformCostVisitor, VertexListGraph> >();

    typedef indirect_cmp<DistanceMap, BinaryPredicate> IndirectCmp;
    IndirectCmp icmp(d, compare);

    typedef mutable_queue<Vertex, std::vector<Vertex>, IndirectCmp, IndexMap>
      MutableQueue;

    MutableQueue Q(num_vertices(g), icmp, id);
    
    detail::ucs_bfs_visitor<UniformCostVisitor, MutableQueue, 
      WeightMap, DistanceMap, BinaryFunction, BinaryPredicate>
      visitor(vis, Q, w, d, combine, compare);

    typename property_traits<ColorMap>::value_type c = get(color, s);
    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
      put(color, *ui, white(c));

    breadth_first_search(g, s, Q, visitor, color);
  }


} // namespace boost

#endif // BOOST_GRAPH_UNIFORM_COST_SEARCH_HPP
