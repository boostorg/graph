//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
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
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/type_traits/same_traits.hpp>
#include <boost/concept/assert.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/graph/detail/traits.hpp>
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/parameter/binding.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif

namespace boost {

  template <class VertexAndEdgeListGraph, class DistanceMatrix,
            class VertexID, class Weight, typename BinaryPredicate, 
            typename BinaryFunction, typename Infinity, class DistanceZero>
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  typename boost::disable_if<
    parameter::are_tagged_arguments<
      VertexID, Weight, BinaryPredicate, BinaryFunction,
      Infinity, DistanceZero
    >,
#endif
    bool
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  >::type
#endif
  johnson_all_pairs_shortest_paths(VertexAndEdgeListGraph& g1, 
               DistanceMatrix& D,
               VertexID id1, Weight w1, const BinaryPredicate& compare, 
               const BinaryFunction& combine, const Infinity& inf,
               DistanceZero zero)
  {
    typedef graph_traits<VertexAndEdgeListGraph> Traits1;
    typedef typename property_traits<Weight>::value_type DT;
    BOOST_CONCEPT_ASSERT(( BasicMatrixConcept<DistanceMatrix,
      typename Traits1::vertices_size_type, DT> ));

    typedef typename Traits1::directed_category DirCat;
    bool is_undirected = is_same<DirCat, undirected_tag>::value;

    typedef adjacency_list<vecS, vecS, directedS, 
      property< vertex_distance_t, DT>,
      property< edge_weight_t, DT, 
      property< edge_weight2_t, DT > > > Graph2;
    typedef graph_traits<Graph2> Traits2;

    Graph2 g2(num_vertices(g1) + 1);
    typename property_map<Graph2, edge_weight_t>::type 
      w = get(edge_weight, g2);
    typename property_map<Graph2, edge_weight2_t>::type 
      w_hat = get(edge_weight2, g2);
    typename property_map<Graph2, vertex_distance_t>::type 
      d = get(vertex_distance, g2);
    typedef typename property_map<Graph2, vertex_index_t>::type VertexID2;
    VertexID2 id2 = get(vertex_index, g2);

    // Construct g2 where V[g2] = V[g1] U {s}
    //   and  E[g2] = E[g1] U {(s,v)| v in V[g1]}
    std::vector<typename Traits1::vertex_descriptor> 
      verts1(num_vertices(g1) + 1);
    typename Traits2::vertex_descriptor s = *vertices(g2).first;
    {
      typename Traits1::vertex_iterator v, v_end;
      int i = 1;
      for (boost::tie(v, v_end) = vertices(g1); v != v_end; ++v, ++i) {
        typename Traits2::edge_descriptor e; bool z;
        boost::tie(e, z) = add_edge(s, get(id1, *v) + 1, g2);
        put(w, e, zero);
        verts1[i] = *v;
      }
      typename Traits1::edge_iterator e, e_end;
      for (boost::tie(e, e_end) = edges(g1); e != e_end; ++e) {
        typename Traits2::edge_descriptor e2; bool z;
        boost::tie(e2, z) = add_edge(get(id1, source(*e, g1)) + 1, 
                                     get(id1, target(*e, g1)) + 1, g2);
        put(w, e2, get(w1, *e));
        if (is_undirected) {
          boost::tie(e2, z) = add_edge(get(id1, target(*e, g1)) + 1, 
                                       get(id1, source(*e, g1)) + 1, g2);
          put(w, e2, get(w1, *e));
        }
      }
    }
    typename Traits2::vertex_iterator v, v_end, u, u_end;
    typename Traits2::edge_iterator e, e_end;
    shared_array_property_map<DT,VertexID2> h(num_vertices(g2), id2);

    for (boost::tie(v, v_end) = vertices(g2); v != v_end; ++v)
      put(d, *v, inf);

    put(d, s, zero);
    // Using the non-named parameter versions of bellman_ford and
    // dijkstra for portability reasons.
    dummy_property_map pred; bellman_visitor<> bvis;
    if (bellman_ford_shortest_paths
        (g2, num_vertices(g2), w, pred, d, combine, compare, bvis)) {
      for (boost::tie(v, v_end) = vertices(g2); v != v_end; ++v)
        put(h, *v, get(d, *v));
      // Reweight the edges to remove negatives
      for (boost::tie(e, e_end) = edges(g2); e != e_end; ++e) {
        typename Traits2::vertex_descriptor a = source(*e, g2),
          b = target(*e, g2);
        put(w_hat, *e, combine((get(h, a) - get(h, b)), get(w, *e)));
      }
      for (boost::tie(u, u_end) = vertices(g2); u != u_end; ++u) {
        dijkstra_visitor<> dvis;
        dijkstra_shortest_paths
          (g2, *u, pred, d, w_hat, id2, compare, combine, inf, zero,dvis);
        for (boost::tie(v, v_end) = vertices(g2); v != v_end; ++v) {
          if (*u != s && *v != s) {
            D[get(id2, *u)-1][get(id2, *v)-1] = combine((get(h, *v) - get(h, *u)), get(d, *v));
          }
        }
      }
      return true;
    } else
      return false;
  }

  template <class VertexAndEdgeListGraph, class DistanceMatrix,
            class VertexID, class Weight, class DistanceZero>
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  typename boost::disable_if<
    parameter::are_tagged_arguments<VertexID, Weight, DistanceZero>,
#endif
    bool
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  >::type
#endif
  johnson_all_pairs_shortest_paths(VertexAndEdgeListGraph& g1, 
               DistanceMatrix& D,
               VertexID id1, Weight w1, DistanceZero zero)
  {
    typedef typename property_traits<Weight>::value_type WT;
    return johnson_all_pairs_shortest_paths(g1, D, id1, w1, 
                                            std::less<WT>(),
                                            closed_plus<WT>(),
                                            (std::numeric_limits<WT>::max)(),
                                            zero);
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <typename VertexAndEdgeListGraph, typename DistanceMatrix,
            typename Args>
  inline typename boost::enable_if<
    parameter::is_argument_pack<Args>, bool
  >::type
  johnson_all_pairs_shortest_paths(VertexAndEdgeListGraph& g1, 
               DistanceMatrix& D, const Args& args)
  {
    using namespace boost::graph::keywords;
    typedef typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::weight_map,
        edge_weight_t,
        VertexAndEdgeListGraph
    >::type weight_map_type;
    typedef typename property_traits<weight_map_type>::value_type WT;
    return johnson_all_pairs_shortest_paths(
      g1,
      D,
      args[
        _vertex_index_map |
        detail::vertex_or_dummy_property_map(g1, vertex_index)
      ],
      args[_weight_map | detail::edge_or_dummy_property_map(g1, edge_weight)],
      args[_distance_compare | std::less<WT>()],
      args[_distance_combine | closed_plus<WT>()],
      args[_distance_inf || detail::get_max<WT>()],
      args[_distance_zero | WT()]
    );
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

  namespace detail {

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
      
      return johnson_all_pairs_shortest_paths
        (g, D, id, w,
        choose_param(get_param(params, distance_compare_t()), 
          std::less<WT>()),
        choose_param(get_param(params, distance_combine_t()), 
          closed_plus<WT>()),
        choose_param(get_param(params, distance_inf_t()), 
          std::numeric_limits<WT>::max BOOST_PREVENT_MACRO_SUBSTITUTION()),
         choose_param(get_param(params, distance_zero_t()), WT()) );
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

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
  template <typename Graph, typename DistanceMatrix, typename TA \
            BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA)> \
  inline typename boost::enable_if< \
    parameter::are_tagged_arguments< \
      TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
    >, bool \
  >::type name( \
    Graph& g, DistanceMatrix& d, const TA& ta \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta) \
  ) \
  { \
    return name(g, d, parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta))); \
  }

BOOST_PP_REPEAT_FROM_TO(1, 7, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, johnson_all_pairs_shortest_paths)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

} // namespace boost

#endif // BOOST_GRAPH_JOHNSON_HPP


