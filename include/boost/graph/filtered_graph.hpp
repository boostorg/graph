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

#ifndef BOOST_FILTERED_GRAPH_HPP
#define BOOST_FILTERED_GRAPH_HPP

#include <boost/pending/iterator_adaptors.hpp>
#include <boost/graph/detail/adjacency_iterator.hpp>

namespace boost {

  //===========================================================================
  // Some predicate classes.

  struct keep_all {
    template <typename T>
    bool operator()(const T&) const { return true; }
  };

  // Keep residual edges (used in maximum-flow algorithms).
  template <typename ResidualCapacityEdgeMap>
  struct is_residual_edge {
    is_residual_edge() { }
    is_residual_edge(ResidualCapacityEdgeMap rcap) : m_rcap(rcap) { }
    template <typename Edge>
    bool operator()(const Edge& e) const {
      return 0 < get(m_rcap, e);
    }
    ResidualCapacityEdgeMap m_rcap;
  };

  namespace detail {
    template <typename EdgePredicate, typename VertexPredicate, typename Graph>
    struct out_edge_predicate {
      out_edge_predicate() { }
      out_edge_predicate(EdgePredicate ep, VertexPredicate vp, 
                         const Graph& g)
        : m_edge_pred(ep), m_vertex_pred(vp), m_g(&g) { }

      template <typename Edge>
      bool operator()(const Edge& e) const {
        return m_edge_pred(e) && m_vertex_pred(target(e, *m_g));
      }
      EdgePredicate m_edge_pred;
      VertexPredicate m_vertex_pred;
      const Graph* m_g;
    };

    template <typename EdgePredicate, typename VertexPredicate, typename Graph>
    struct in_edge_predicate {
      in_edge_predicate() { }
      in_edge_predicate(EdgePredicate ep, VertexPredicate vp, 
                         const Graph& g)
        : m_edge_pred(ep), m_vertex_pred(vp), m_g(&g) { }

      template <typename Edge>
      bool operator()(const Edge& e) const {
        return m_edge_pred(e) && m_vertex_pred(source(e, *m_g));
      }
      EdgePredicate m_edge_pred;
      VertexPredicate m_vertex_pred;
      const Graph* m_g;
    };

    template <typename EdgePredicate, typename VertexPredicate, typename Graph>
    struct edge_predicate {
      edge_predicate() { }
      edge_predicate(EdgePredicate ep, VertexPredicate vp, 
		     const Graph& g)
        : m_edge_pred(ep), m_vertex_pred(vp), m_g(&g) { }

      template <typename Edge>
      bool operator()(const Edge& e) const {
        return m_edge_pred(e)
	  && m_vertex_pred(source(e, *m_g)) && m_vertex_pred(target(e, *m_g));
      }
      EdgePredicate m_edge_pred;
      VertexPredicate m_vertex_pred;
      const Graph* m_g;
    };
  } // namespace detail


  //===========================================================================
  // Filtered Graph

  template <typename Graph, 
            typename EdgePredicate,
            typename VertexPredicate = keep_all>
  class filtered_graph {
    typedef graph_traits<Graph> Traits;
    typedef filtered_graph self;
  public:
    typedef detail::out_edge_predicate<EdgePredicate, 
      VertexPredicate, self> OutEdgePred;
    typedef detail::in_edge_predicate<EdgePredicate, 
      VertexPredicate, self> InEdgePred;
    typedef detail::edge_predicate<EdgePredicate, 
      VertexPredicate, self> EdgePred;

    // Constructors
    filtered_graph(Graph& g, EdgePredicate ep)
      : m_g(g), m_edge_pred(ep) { }

    filtered_graph(Graph& g, EdgePredicate ep, VertexPredicate vp)
      : m_g(g), m_edge_pred(ep), m_vertex_pred(vp) { }

    // Graph requirements
    typedef typename Traits::vertex_descriptor          vertex_descriptor;
    typedef typename Traits::edge_descriptor            edge_descriptor;
    typedef typename Traits::directed_category          directed_category;
    typedef typename Traits::edge_parallel_category     edge_parallel_category;

    // IncidenceGraph requirements
    typedef typename filter_iterator<OutEdgePred,
      typename Traits::out_edge_iterator,
      boost::iterator<multi_pass_input_iterator_tag, 
        edge_descriptor, std::ptrdiff_t, 
        edge_descriptor*, edge_descriptor> >::type     out_edge_iterator;
    typedef typename Traits::degree_size_type          degree_size_type;

    // AdjacencyGraph requirements
    typedef typename detail::adjacency_iterator<
       self, vertex_descriptor, out_edge_iterator, 
       out_edge_iterator>::type                        adjacency_iterator;

    // BidirectionalGraph requirements
    typedef typename filter_iterator<InEdgePred,
      typename Traits::in_edge_iterator,
      boost::iterator<multi_pass_input_iterator_tag, 
        edge_descriptor, std::ptrdiff_t, 
        edge_descriptor*, edge_descriptor> >::type     in_edge_iterator;

    // VertexListGraph requirements
    typedef typename filter_iterator<VertexPredicate,
      typename Traits::vertex_iterator,
      boost::iterator<multi_pass_input_iterator_tag,
      vertex_descriptor, std::ptrdiff_t,
      vertex_descriptor*, vertex_descriptor> >::type   vertex_iterator;
    typedef typename Traits::vertices_size_type        vertices_size_type;

    // EdgeListGraph requirements
    typedef typename filter_iterator<EdgePred,
      typename Traits::edge_iterator,
      boost::iterator<multi_pass_input_iterator_tag, 
        edge_descriptor, std::ptrdiff_t,
        edge_descriptor*, edge_descriptor> >::type     edge_iterator;
    typedef typename Traits::edges_size_type edges_size_type;

    typedef typename Graph::edge_property_type edge_property_type;
    typedef typename Graph::vertex_property_type vertex_property_type;
    typedef typename Graph::graph_tag graph_tag;

    //private:
    Graph& m_g;
    EdgePredicate m_edge_pred;
    VertexPredicate m_vertex_pred;

    typedef filter_iterator_policies<OutEdgePred,
      typename Traits::out_edge_iterator> out_edge_iter_policy;
    typedef filter_iterator_policies<InEdgePred,
      typename Traits::in_edge_iterator> in_edge_iter_policy;
    typedef filter_iterator_policies<EdgePred,
      typename Traits::edge_iterator> edge_iter_policy;
  };

  //===========================================================================
  // Non-member functions for the Filtered Edge Graph

  // Helper functions
  template <typename Graph, typename EdgePredicate>
  inline filtered_graph<Graph, EdgePredicate>
  make_filtered_graph(Graph& g, EdgePredicate ep) {
    return filtered_graph<Graph, EdgePredicate>(g, ep);
  }
  template <typename Graph, typename EdgePredicate, typename VertexPredicate>
  inline filtered_graph<Graph, EdgePredicate, VertexPredicate>
  make_filtered_graph(Graph& g, EdgePredicate ep, VertexPredicate vp) {
    return filtered_graph<Graph, EdgePredicate>(g, ep, vp);
  }

  template <typename G, typename EP, typename VP>
  std::pair<typename graph_traits<G>::vertex_iterator,
            typename graph_traits<G>::vertex_iterator>
  vertices(const filtered_graph<G, EP, VP>& g) {
    return vertices(g.m_g);
  }

  template <typename G, typename EP, typename VP>
  std::pair<typename filtered_graph<G, EP, VP>::edge_iterator,
            typename filtered_graph<G, EP, VP>::edge_iterator>
  edges(const filtered_graph<G, EP, VP>& g) {
    typedef filtered_graph<G, EP, VP> Graph;
    typedef typename Graph::edge_iter_policy Pol;
    typename Graph::EdgePred pred(g.m_edge_pred, g.m_vertex_pred, g);
    typename graph_traits<G>::edge_iterator f, l;
    tie(f, l) = edges(g.m_g);
    typedef typename Graph::edge_iterator iter;
    return std::make_pair(iter(f, Pol(pred, l)), 
                          iter(l, Pol(pred, l)));
  }

  template <typename G, typename EP, typename VP>  
  typename graph_traits<G>::vertices_size_type
  num_vertices(const filtered_graph<G, EP, VP>& g) {
    return num_vertices(g);
  }

  template <typename G, typename EP, typename VP>  
  typename graph_traits<G>::edges_size_type
  num_edges(const filtered_graph<G, EP, VP>& g) {
    return num_edges(g);
  }
  
  template <typename G, typename EP, typename VP>
  typename graph_traits<G>::vertex_descriptor
  source(typename graph_traits<G>::edge_descriptor e,
         const filtered_graph<G, EP, VP>& g)
  {
    return source(e, g.m_g);
  }

  template <typename G, typename EP, typename VP>
  typename graph_traits<G>::vertex_descriptor
  target(typename graph_traits<G>::edge_descriptor e,
         const filtered_graph<G, EP, VP>& g)
  {
    return target(e, g.m_g);
  }

  template <typename G, typename EP, typename VP>
  std::pair<typename filtered_graph<G, EP, VP>::out_edge_iterator,
            typename filtered_graph<G, EP, VP>::out_edge_iterator>
  out_edges(typename graph_traits<G>::vertex_descriptor u,
            const filtered_graph<G, EP, VP>& g)
  {
    typedef filtered_graph<G, EP, VP> Graph;
    typename Graph::OutEdgePred pred(g.m_edge_pred, g.m_vertex_pred, g);
    typedef typename Graph::out_edge_iter_policy Pol;
    typedef typename Graph::out_edge_iterator iter;
    typename graph_traits<G>::out_edge_iterator f, l;
    tie(f, l) = out_edges(u, g.m_g);
    return std::make_pair(iter(f, Pol(pred, l)), iter(l, Pol(pred, l)));
  }

  template <typename G, typename EP, typename VP>
  typename graph_traits<G>::degree_size_type
  out_degree(typename graph_traits<G>::vertex_descriptor u,
             const filtered_graph<G, EP, VP>& g)
  {
    typename graph_traits<G>::degree_size_type n = 0;
    typename filtered_graph<G, EP, VP>::out_edge_iterator f, l;
    for (tie(f, l) = out_edges(u, g); f != l; ++f)
      ++n;
    return n;
  }

  template <typename G, typename EP, typename VP>
  std::pair<typename filtered_graph<G, EP, VP>::adjacency_iterator,
            typename filtered_graph<G, EP, VP>::adjacency_iterator>
  adjacent_vertices(typename graph_traits<G>::vertex_descriptor u,
                    const filtered_graph<G, EP, VP>& g)
  {
    typedef filtered_graph<G, EP, VP> Graph;
    typedef typename Graph::adjacency_iterator adjacency_iterator;
    typename Graph::out_edge_iterator f, l;
    tie(f, l) = out_edges(u, g);
    return std::make_pair(adjacency_iterator(f, const_cast<Graph*>(&g)),
                          adjacency_iterator(l, const_cast<Graph*>(&g)));
  }
  
  template <typename G, typename EP, typename VP>
  std::pair<typename filtered_graph<G, EP, VP>::in_edge_iterator,
            typename filtered_graph<G, EP, VP>::in_edge_iterator>
  in_edges(typename graph_traits<G>::vertex_descriptor u,
            const filtered_graph<G, EP, VP>& g)
  {
    typedef filtered_graph<G, EP, VP> Graph;
    typename Graph::InEdgePred pred(g.m_edge_pred, g.m_vertex_pred, g);
    typedef typename Graph::in_edge_iter_policy Pol;
    typedef typename Graph::in_edge_iterator iter;
    typename graph_traits<G>::in_edge_iterator f, l;
    tie(f, l) = in_edges(u, g.m_g);
    return std::make_pair(iter(f, Pol(pred, l)), iter(l, Pol(pred, l)));
  }

  template <typename G, typename EP, typename VP>
  std::pair<typename filtered_graph<G, EP, VP>::edge_descriptor, bool>
  edge(typename graph_traits<G>::vertex_descriptor u,
       typename graph_traits<G>::vertex_descriptor v,
       const filtered_graph<G, EP, VP>& g)
  {
    typename G::edge_descriptor e;
    bool exists;
    tie(e, exists) = edge(u, v, g.m_g);
    return std::make_pair(e, exists && g.m_edge_pred(e));
  }

  template <typename G, typename EP, typename VP, typename Property>
  typename property_map<G, Property>::type
  get(Property p, filtered_graph<G, EP, VP>& g)
  {
    return get(p, g.m_g);
  }

  template <typename G, typename EP, typename VP,typename Property>
  typename property_map<G, Property>::const_type
  get(Property p, const filtered_graph<G, EP, VP>& g)
  {
    return get(p, g.m_g);
  }

  template <typename G, typename EP, typename VP, typename Property,
            typename Key>
  typename property_traits<
    typename property_map<G, Property>::const_type
  >::value_type
  get(Property p, const filtered_graph<G, EP, VP>& g, const Key& k)
  {
    return get(p, g.m_g, k);
  }

  template <typename G, typename EP, typename VP, typename Property, 
            typename Key, typename Value>
  void
  put(Property p, const filtered_graph<G, EP, VP>& g, const Key& k,
      const Value& val)
  {
    put(p, g.m_g, k, val);
  }


} // namespace boost


#endif // BOOST_FILTERED_GRAPH_HPP
