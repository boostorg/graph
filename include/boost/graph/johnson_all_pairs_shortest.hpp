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

/*
  This file implements the function

  template <class VertexAndEdgeListGraph, class DistanceMatrix,
            class P, class T, class R>
  bool
  johnson_all_pairs_shortest_paths
    (VertexAndEdgeListGraph& g, 
     DistanceMatrix& D,
     const bgl_named_params<P, T, R>& params)
 */

#ifndef BOOST_GRAPH_JOHNSON_HPP
#define BOOST_GRAPH_JOHNSON_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace boost {

  namespace detail {

    template <class VertexAndEdgeListGraph, class DistanceMatrix,
              class P, class T, class R, class VertexID, class Distance,
              class Weight, class Weight2, class DistanceZero>
    bool
    johnson_impl(VertexAndEdgeListGraph& g, 
                 DistanceMatrix& D,
                 const bgl_named_params<P, T, R>& params,
                 VertexID id, Distance d, Weight w, Weight2 w_hat,
                 DistanceZero zero)
    {
      typedef graph_traits<VertexAndEdgeListGraph> Traits;
      typename Traits::vertex_iterator v, v_end, u, u_end;
      typename Traits::edge_iterator e, e_end;
      typename Traits::vertex_descriptor s = *vertices(g).first;
      typedef typename property_traits<Distance>::value_type DT;
      function_requires< BasicMatrixConcept<DistanceMatrix,
        typename Traits::vertex_descriptor, DT> >();
      
      std::vector<DT> h_vec(num_vertices(g));
      typedef typename std::vector<DT>::iterator iter_t;
      iterator_property_map<iter_t, VertexID, DT, DT&> h(h_vec.begin(), id);

      put(d, s, zero);
      if (bellman_ford_shortest_paths(g, num_vertices(g), params)) {
        for (tie(v, v_end) = vertices(g); v != v_end; ++v)
          put(h, *v, get(d, *v));
        for (tie(e, e_end) = edges(g); e != e_end; ++e)
          put(w_hat, *e, 
              get(w, *e) + get(h, source(*e,g)) - get(h, target(*e,g)));
        for (tie(u, u_end) = vertices(g); u != u_end; ++u) {
          dijkstra_shortest_paths(g, *u, params.weight_map(w_hat));
          for (tie(v, v_end) = vertices(g); v != v_end; ++v)
            D[*u][*v] = get(d, *v) + get(h, *v) - get(h, *u);
        }
        return true;
      } else
        return false;
    }
    
    template <class VertexAndEdgeListGraph, class DistanceMatrix,
              class P, class T, class R, class Weight, 
              class VertexID>
    bool
    johnson_dispatch(VertexAndEdgeListGraph& g, 
                     DistanceMatrix& D,
                     const bgl_named_params<P, T, R>& params,
                     Weight w, VertexID id)
    {
      typedef typename property_traits<Weight>::value_type WT;
      typename std::vector<WT>::size_type 
        n = is_default_param(get_param(params, vertex_distance))
        ? num_vertices(g) : 1;
      std::vector<WT> distance_map(n);
      
      return detail::johnson_impl
        (g, D, params, id,
         choose_param(get_param(params, vertex_distance),
                      make_iterator_property_map
                      (distance_map.begin(), id, distance_map[0])),
         w,
         choose_pmap(get_param(params, edge_weight2), g, edge_weight2),
         choose_param(get_param(params, distance_zero_t()), 
                      WT()) );
    }

  } // namespace detail

  template <class VertexAndEdgeListGraph, class DistanceMatrix,
            class P, class T, class R>
  bool
  johnson_all_pairs_shortest_paths
    (VertexAndEdgeListGraph& g, 
     DistanceMatrix& D,
     const bgl_named_params<P, T, R>& params)
  {
    return detail::johnson_dispatch
      (g, D, params,
       choose_const_pmap(get_param(params, edge_weight), g, edge_weight),
       choose_const_pmap(get_param(params, vertex_index), g, vertex_index)
       );
  }

  template <class VertexAndEdgeListGraph, class DistanceMatrix>
  bool
  johnson_all_pairs_shortest_paths
    (VertexAndEdgeListGraph& g, DistanceMatrix& D)
  {
    bgl_named_params<int,int> params(1);
    return detail::johnson_dispatch
      (g, D, params, get(edge_weight, g), get(vertex_index, g));
  }


} // namespace boost

#endif // BOOST_GRAPH_JOHNSON_HPP


