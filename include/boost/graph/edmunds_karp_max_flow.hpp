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
#include <boost/graph/filtered_edge_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>

namespace boost {

  // The "labeling" algorithm from "Network Flows" by Ahuja, Magnanti, Orlin.
  // I think it is the same as or similar to the Edmunds-Karp algorithm.
  // This solves the maximum flow problem.

  namespace detail {

    template <class Graph, class ResCapMap>
    filtered_edge_graph<Graph, is_residual_edge<ResCapMap> >
    residual_graph(Graph& g, ResCapMap residual_capacity) {
      return filtered_edge_graph<Graph, is_residual_edge<ResCapMap> >
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

  } // namespace detail

  template <class Graph, 
            class CapacityEdgeMap, class ResidualCapacityEdgeMap,
            class ReverseEdgeMap, class ColorMap, class PredEdgeMap>
  typename property_traits<CapacityEdgeMap>::value_type
  edmunds_karp_max_flow
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

    typedef color_traits<typename property_traits<ColorMap>::value_type> Color;

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
  } // edmunds_karp_max_flow()

  template <class Graph, 
            class CapacityEdgeMap, class ResidualCapacityEdgeMap,
            class ReverseEdgeMap, class VertexIndexMap>
  typename property_traits<CapacityEdgeMap>::value_type
  edmunds_karp_max_flow
    (Graph& g, 
     typename graph_traits<Graph>::vertex_descriptor src,
     typename graph_traits<Graph>::vertex_descriptor sink,
     CapacityEdgeMap cap, 
     ResidualCapacityEdgeMap res,
     ReverseEdgeMap rev, 
     VertexIndexMap index_map)
  {
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
    std::vector<edge_descriptor> pred(num_vertices(g));
    std::vector<default_color_type> color(num_vertices(g));

    return edmunds_karp_max_flow
      (g, src, sink, cap, res, rev, 
       make_iterator_property_map(color.begin(), index_map, color[0]), 
       make_iterator_property_map(pred.begin(), index_map, pred[0]));
  } // edmunds_karp_max_flow()

} // namespace boost

#endif // EDMUNDS_KARP_MAX_FLOW_HPP
