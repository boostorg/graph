//=======================================================================
// Copyright 2000 University of Notre Dame.
// Authors: Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee
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

#ifndef EDMUNDS_KARP_MAX_FLOW_HPP
#define EDMUNDS_KARP_MAX_FLOW_HPP

#include <boost/config.hpp>
#include <vector>
#include <boost/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>

namespace boost {

  // The "labeling" algorithm from "Network Flows" by Ahuja, Magnanti,
  // Orlin.  I think this is the same as or very similar to the original
  // Edmunds-Karp algorithm.  This solves the maximum flow problem.

  namespace detail {

    template <class Graph, class ResCapMap>
    filtered_graph<Graph, is_residual_edge<ResCapMap> >
    residual_graph(Graph& g, ResCapMap residual_capacity) {
      return filtered_graph<Graph, is_residual_edge<ResCapMap> >
        (g, is_residual_edge<ResCapMap>(residual_capacity));
    }

    template <class Graph, class PredEdgeMap, class ResCapMap,
              class RevEdgeMap>
    inline void
    augment(Graph& g, 
            typename graph_traits<Graph>::vertex_descriptor src,
            typename graph_traits<Graph>::vertex_descriptor sink,
            PredEdgeMap p, 
            ResCapMap residual_capacity,
            RevEdgeMap reverse_edge)
    {
      typename graph_traits<Graph>::edge_descriptor e;
      typename graph_traits<Graph>::vertex_descriptor u;
      typedef typename property_traits<ResCapMap>::value_type FlowValue;

      // find minimum residual capacity along the augmenting path
      FlowValue delta = std::numeric_limits<FlowValue>::max();
      e = p[sink];
      do {
        delta = std::min(delta, residual_capacity[e]);
        u = source(e, g);
        e = p[u];
      } while (u != src);

      // push delta units of flow along the augmenting path
      e = p[sink];
      do {
        residual_capacity[e] -= delta;
        residual_capacity[reverse_edge[e]] += delta;
        u = source(e, g);
        e = p[u];
      } while (u != src);
    }

    template <class Graph, class P, class T, class R>
    struct edge_capacity_value
    {
      typedef bgl_named_params<P, T, R> Params;
      typedef typename property_value< Params, edge_capacity_t>::type Param;
      typedef typename detail::choose_pmap_helper<Param, Graph,
	edge_capacity_t>::result CapacityEdgeMap;
      typedef typename property_traits<CapacityEdgeMap>::value_type type;
    };

    template <class Graph, 
	      class CapacityEdgeMap, class ResidualCapacityEdgeMap,
	      class ReverseEdgeMap, class ColorMap, class PredEdgeMap>
    typename property_traits<CapacityEdgeMap>::value_type
    edmunds_karp_max_flow_impl
      (Graph& g, 
       typename graph_traits<Graph>::vertex_descriptor src,
       typename graph_traits<Graph>::vertex_descriptor sink,
       CapacityEdgeMap cap, 
       ResidualCapacityEdgeMap res,
       ReverseEdgeMap rev, 
       ColorMap color, 
       PredEdgeMap pred)
    {
      typename graph_traits<Graph>::vertex_iterator u_iter, u_end;
      typename graph_traits<Graph>::out_edge_iterator ei, e_end;
      for (tie(u_iter, u_end) = vertices(g); u_iter != u_end; ++u_iter)
	for (tie(ei, e_end) = out_edges(*u_iter, g); ei != e_end; ++ei)
	  res[*ei] = cap[*ei];

      typedef color_traits<typename property_traits<ColorMap>::value_type> 
	Color;

      color[sink] = Color::gray();
      while (color[sink] != Color::white()) {
	breadth_first_search
	  (detail::residual_graph(g, res), src,
	   make_bfs_visitor(record_edge_predecessors(pred, on_tree_edge())),
	   color);
	if (color[sink] != Color::white())
	  detail::augment(g, src, sink, pred, res, rev);
      } // while

      typename property_traits<CapacityEdgeMap>::value_type flow = 0;
      for (tie(ei, e_end) = out_edges(src, g); ei != e_end; ++ei)
	flow += (cap[*ei] - res[*ei]);
      return flow;
    } // edmunds_karp_max_flow_impl()
    
    //-------------------------------------------------------------------------
    // Handle default for color property map

    template <class Graph, class PredMap, class P, class T, class R>
    typename edge_capacity_value<Graph, P, T, R>::type
    edmunds_karp_dispatch2
      (Graph& g,
       typename graph_traits<Graph>::vertex_descriptor src,
       typename graph_traits<Graph>::vertex_descriptor sink,
       PredMap pred,
       const bgl_named_params<P, T, R>& params,
       detail::error_property_not_found)
    {
      typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
      typedef typename graph_traits<Graph>::vertices_size_type size_type;
      size_type n = is_default_param(get_param(params, vertex_color)) ?
	num_vertices(g) : 0;
      std::vector<default_color_type> color_vec(n);
      return edmunds_karp_max_flow_impl
	(g, src, sink, 
	 choose_const_pmap(get_param(params, edge_capacity), g, edge_capacity),
	 choose_pmap(get_param(params, edge_residual_capacity), 
		     g, edge_residual_capacity),
	 choose_const_pmap(get_param(params, edge_reverse), g, edge_reverse),
	 make_iterator_property_map(color_vec.begin(), choose_const_pmap
				      (get_param(params, vertex_index),
				       g, vertex_index)),
	 pred);
    }

    template <class Graph, class PredMap, class P, class T, class R, 
              class ColorMap>
    typename edge_capacity_value<Graph, P, T, R>::type
    edmunds_karp_dispatch2
      (Graph& g,
       typename graph_traits<Graph>::vertex_descriptor src,
       typename graph_traits<Graph>::vertex_descriptor sink,
       PredMap pred,
       const bgl_named_params<P, T, R>& params,
       ColorMap color)
    {
      return edmunds_karp_max_flow_impl
	(g, src, sink, 
	 choose_const_pmap(get_param(params, edge_capacity), g, edge_capacity),
	 choose_pmap(get_param(params, edge_residual_capacity), 
		     g, edge_residual_capacity),
	 choose_const_pmap(get_param(params, edge_reverse), g, edge_reverse),
	 color, pred);
    }

    //-------------------------------------------------------------------------
    // Handle default for predecessor property map

    template <class Graph, class P, class T, class R>
    typename edge_capacity_value<Graph, P, T, R>::type
    edmunds_karp_dispatch1
      (Graph& g,
       typename graph_traits<Graph>::vertex_descriptor src,
       typename graph_traits<Graph>::vertex_descriptor sink,
       const bgl_named_params<P, T, R>& params,
       detail::error_property_not_found)
    {
      typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
      typedef typename graph_traits<Graph>::vertices_size_type size_type;
      size_type n = is_default_param(get_param(params, vertex_predecessor)) ?
	num_vertices(g) : 0;
      std::vector<edge_descriptor> pred_vec(n);

      return edmunds_karp_dispatch2
	(g, src, sink, 
	 make_iterator_property_map(pred_vec.begin(), choose_const_pmap
				    (get_param(params, vertex_index),
				     g, vertex_index)),
	 params, 
	 get_param(params, vertex_color));
    }

    template <class Graph, class P, class T, class R, class PredMap>
    typename edge_capacity_value<Graph, P, T, R>::type
    edmunds_karp_dispatch1
      (Graph& g,
       typename graph_traits<Graph>::vertex_descriptor src,
       typename graph_traits<Graph>::vertex_descriptor sink,
       const bgl_named_params<P, T, R>& params,
       PredMap pred)
    {
      typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
      typedef typename graph_traits<Graph>::vertices_size_type size_type;
      size_type n = is_default_param(get_param(params, vertex_predecessor)) ?
	num_vertices(g) : 0;
      std::vector<edge_descriptor> pred_vec(n);

      return edmunds_karp_dispatch2
	(g, src, sink, pred, params, get_param(params, vertex_color));
    }
    
  } // namespace detail

  template <class Graph, class P, class T, class R>
  typename detail::edge_capacity_value<Graph, P, T, R>::type
  edmunds_karp_max_flow
    (Graph& g,
     typename graph_traits<Graph>::vertex_descriptor src,
     typename graph_traits<Graph>::vertex_descriptor sink,
     const bgl_named_params<P, T, R>& params)
  {
    return detail::edmunds_karp_dispatch1
      (g, src, sink, params, get_param(params, vertex_predecessor));
  }

  template <class Graph>
  typename property_traits<
    typename property_map<Graph, edge_capacity_t>::const_type
  >::value_type
  edmunds_karp_max_flow
    (Graph& g,
     typename graph_traits<Graph>::vertex_descriptor src,
     typename graph_traits<Graph>::vertex_descriptor sink)
  {
    bgl_named_params<int, int> params(0);
    return edmunds_karp_max_flow(g, src, sink, params);
  }

} // namespace boost

#endif // EDMUNDS_KARP_MAX_FLOW_HPP
