//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_MST_PRIM_HPP
#define BOOST_GRAPH_MST_PRIM_HPP

#include <functional>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace boost {
  
  namespace detail {
    // this should be somewhere else in boost...
    template <class U, class V> struct _project2nd {
      V operator()(U, V v) const { return v; }
    };
  }

  namespace detail {

    // This is Prim's algorithm to calculate the Minimum Spanning Tree
    // for an undirected graph with weighted edges.

    template <class Graph, class P, class T, class R, class Weight>
    inline void
    prim_mst_impl(const Graph& G,
                  typename graph_traits<Graph>::vertex_descriptor s,
                  const bgl_named_params<P,T,R>& params,
                  Weight)
    {
      typedef typename property_traits<Weight>::value_type W;
      std::less<W> compare;
      detail::_project2nd<W,W> combine;
      dijkstra_shortest_paths(G, s, params.distance_compare(compare).
                              distance_combine(combine));
    }
  } // namespace detail

  template <class VertexListGraph, class DijkstraVisitor, 
            class PredecessorMap, class DistanceMap,
            class WeightMap, class IndexMap>
  inline void
  prim_minimum_spanning_tree
    (const VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s, 
     PredecessorMap predecessor, DistanceMap distance, WeightMap weight, 
     IndexMap index_map, DijkstraVisitor vis, typename boost::disable_if<
       parameter::are_tagged_arguments<
         PredecessorMap, DistanceMap, WeightMap, IndexMap, DijkstraVisitor
       >,
       mpl::true_
     >::type = mpl::true_())
  {
    typedef typename property_traits<WeightMap>::value_type W;
    std::less<W> compare;
    detail::_project2nd<W,W> combine;
    dijkstra_shortest_paths(g, s, predecessor, distance, weight, index_map,
                            compare, combine, (std::numeric_limits<W>::max)(), 0,
                            vis);
  }

  template <class VertexListGraph, class PredecessorMap,
            class P, class T, class R>
  inline void prim_minimum_spanning_tree
    (const VertexListGraph& g,
     PredecessorMap p_map,
     const bgl_named_params<P,T,R>& params)
  {
    detail::prim_mst_impl
      (g, 
       choose_param(get_param(params, root_vertex_t()), *vertices(g).first), 
       params.predecessor_map(p_map),
       choose_const_pmap(get_param(params, edge_weight), g, edge_weight));
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <class VertexListGraph, class PredecessorMap, class Args>
  inline void prim_minimum_spanning_tree
    (const VertexListGraph& g, PredecessorMap p_map,
     const Args& arg_pack, typename boost::enable_if<
       parameter::is_argument_pack<Args>, mpl::true_
     >::type = mpl::true_())
  {
    using namespace boost::graph::keywords;
    typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::vertex_index_map,
        vertex_index_t,
        VertexListGraph
    >::type v_i_map = detail::override_const_property(
        arg_pack,
        _vertex_index_map,
        g,
        vertex_index
    );
    typedef typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::weight_map,
        edge_weight_t,
        VertexListGraph
    >::type weight_map_type;
    typedef typename boost::property_traits<weight_map_type>::value_type W;
    const W zero_actual = W();
    boost::detail::make_property_map_from_arg_pack_gen<
        boost::graph::keywords::tag::distance_map,
        W
    > dist_map_gen(zero_actual);
    typename boost::detail::map_maker<
        VertexListGraph,
        Args,
        boost::graph::keywords::tag::distance_map,
        W
    >::map_type dist_map = dist_map_gen(g, arg_pack);
    weight_map_type w_map = detail::override_const_property(
        arg_pack,
        _weight_map,
        g,
        edge_weight
    );
    std::less<W> compare;
    detail::_project2nd<W,W> combine;
    null_visitor null_vis;
    dijkstra_visitor<null_visitor> default_visitor(null_vis);
    typename boost::parameter::binding<
        Args, 
        boost::graph::keywords::tag::visitor,
        dijkstra_visitor<null_visitor>&
    >::type vis = arg_pack[_visitor | default_visitor];
    dijkstra_shortest_paths(
      g,
      arg_pack[
        _root_vertex ||
        detail::get_default_starting_vertex_t<VertexListGraph>(g)
      ],
      _predecessor_map = p_map,
      _distance_map = dist_map,
      _weight_map = w_map,
      _vertex_index_map = v_i_map,
      _distance_compare = compare,
      _distance_combine = combine,
      _distance_inf = (std::numeric_limits<W>::max)(),
      _visitor = vis,
      _color_map = arg_pack[
        _color_map | make_two_bit_color_map(num_vertices(g), v_i_map)
      ]
    );
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

  template <class VertexListGraph, class PredecessorMap>
  inline void prim_minimum_spanning_tree
    (const VertexListGraph& g, PredecessorMap p_map)
  {
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    prim_minimum_spanning_tree(g, p_map, parameter::compose());
#else
    detail::prim_mst_impl
      (g, *vertices(g).first, predecessor_map(p_map).
       weight_map(get(edge_weight, g)),
       get(edge_weight, g));
#endif
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
  template <typename Graph, typename PredMap, typename TA \
            BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA)> \
  inline void name \
    (const Graph &g, PredMap p_map, \
     const TA& ta BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
     typename boost::enable_if< \
       parameter::are_tagged_arguments< \
         TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
       >, mpl::true_ \
     >::type = mpl::true_()) \
  { \
    name(g, p_map, parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta))); \
  }

BOOST_PP_REPEAT_FROM_TO(1, 6, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, prim_minimum_spanning_tree)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

} // namespace boost

#endif // BOOST_GRAPH_MST_PRIM_HPP
