//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

/*
  This file implements the function

  template <class EdgeListGraph, class Size, class P, class T, class R>
  bool bellman_ford_shortest_paths(EdgeListGraph& g, Size N, 
     const bgl_named_params<P, T, R>& params)
  
 */


#ifndef BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP
#define BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/relax.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/concept/assert.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/graph/detail/traits.hpp>
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/parameter/binding.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif

namespace boost {

  template <class Visitor, class Graph>
  struct BellmanFordVisitorConcept {
    void constraints() {
      BOOST_CONCEPT_ASSERT(( CopyConstructibleConcept<Visitor> ));
      vis.examine_edge(e, g);
      vis.edge_relaxed(e, g);
      vis.edge_not_relaxed(e, g);
      vis.edge_minimized(e, g);
      vis.edge_not_minimized(e, g);
    }
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::edge_descriptor e;
  };

  template <class Visitors = null_visitor>
  class bellman_visitor {
  public:
    bellman_visitor() { }
    bellman_visitor(Visitors vis) : m_vis(vis) { }

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
  typedef bellman_visitor<> default_bellman_visitor;

  template <class EdgeListGraph, class Size, class WeightMap,
            class PredecessorMap, class DistanceMap,
            class BinaryFunction, class BinaryPredicate,
            class BellmanFordVisitor>
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  typename boost::disable_if<
    parameter::are_tagged_arguments<
      Size, WeightMap, PredecessorMap, DistanceMap,
      BinaryFunction, BinaryPredicate, BellmanFordVisitor
    >,
#endif
    bool
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  >::type
#endif
  bellman_ford_shortest_paths(EdgeListGraph& g, Size N, 
                         WeightMap weight, 
                         PredecessorMap pred,
                         DistanceMap distance, 
                         BinaryFunction combine, 
                         BinaryPredicate compare,
                         BellmanFordVisitor v)
  {
    BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<EdgeListGraph> ));
    typedef graph_traits<EdgeListGraph> GTraits;
    typedef typename GTraits::edge_descriptor Edge;
    typedef typename GTraits::vertex_descriptor Vertex;
    BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<DistanceMap, Vertex> ));
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<WeightMap, Edge> ));

    typename GTraits::edge_iterator i, end;

    for (Size k = 0; k < N; ++k) {
      bool at_least_one_edge_relaxed = false;
      for (boost::tie(i, end) = edges(g); i != end; ++i) {
        v.examine_edge(*i, g);
        if (relax(*i, g, weight, pred, distance, combine, compare)) {
          at_least_one_edge_relaxed = true;
          v.edge_relaxed(*i, g);
        } else
          v.edge_not_relaxed(*i, g);
      }
      if (!at_least_one_edge_relaxed)
        break;
    }

    for (boost::tie(i, end) = edges(g); i != end; ++i)
      if (compare(combine(get(distance, source(*i, g)), get(weight, *i)),
                  get(distance, target(*i,g))))
      {
        v.edge_not_minimized(*i, g);
        return false;
      } else
        v.edge_minimized(*i, g);

    return true;
  }

  namespace detail {

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    template <
      typename Graph, typename WeightMap, typename DistanceMap,
      typename Args
    >
    void bellman_initialize_distance_map(
      Graph& g,
      WeightMap,
      DistanceMap distance,
      const Args& arg_pack,
      mpl::true_
    )
    {
      typedef typename property_traits<WeightMap>::value_type weight_type;
      typename graph_traits<Graph>::vertex_iterator v, v_end;
      for (boost::tie(v, v_end) = vertices(g); v != v_end; ++v) {
        put(distance, *v, (std::numeric_limits<weight_type>::max)());
      }
      put(distance, arg_pack[root_vertex], weight_type(0));
    }

    template <
      typename Graph, typename WeightMap, typename DistanceMap,
      typename Args
    >
    inline void bellman_initialize_distance_map(
      Graph&,
      WeightMap,
      DistanceMap,
      const Args&,
      mpl::false_
    )
    {
    }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

    template<typename VertexAndEdgeListGraph, typename Size, 
             typename WeightMap, typename PredecessorMap, typename DistanceMap,
             typename P, typename T, typename R>
    bool 
    bellman_dispatch2
      (VertexAndEdgeListGraph& g, 
       typename graph_traits<VertexAndEdgeListGraph>::vertex_descriptor s,
       Size N, WeightMap weight, PredecessorMap pred, DistanceMap distance,
       const bgl_named_params<P, T, R>& params)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      bellman_visitor<> null_vis;
      typedef typename property_traits<WeightMap>::value_type weight_type;
      typename graph_traits<VertexAndEdgeListGraph>::vertex_iterator v, v_end;
      for (boost::tie(v, v_end) = vertices(g); v != v_end; ++v) {
        put(distance, *v, (std::numeric_limits<weight_type>::max)());
        put(pred, *v, *v);
      }
      put(distance, s, weight_type(0));
      return bellman_ford_shortest_paths
               (g, N, weight, pred, distance,
                choose_param(get_param(params, distance_combine_t()),
                             closed_plus<D>()),
                choose_param(get_param(params, distance_compare_t()),
                             std::less<D>()),
                choose_param(get_param(params, graph_visitor),
                             null_vis)
                );
    }

    template<typename VertexAndEdgeListGraph, typename Size, 
             typename WeightMap, typename PredecessorMap, typename DistanceMap,
             typename P, typename T, typename R>
    bool 
    bellman_dispatch2
      (VertexAndEdgeListGraph& g, 
       param_not_found,
       Size N, WeightMap weight, PredecessorMap pred, DistanceMap distance,
       const bgl_named_params<P, T, R>& params)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      bellman_visitor<> null_vis;
      return bellman_ford_shortest_paths
               (g, N, weight, pred, distance,
                choose_param(get_param(params, distance_combine_t()),
                             closed_plus<D>()),
                choose_param(get_param(params, distance_compare_t()),
                             std::less<D>()),
                choose_param(get_param(params, graph_visitor),
                             null_vis)
                );
    }

    template <class EdgeListGraph, class Size, class WeightMap,
              class DistanceMap, class P, class T, class R>
    bool bellman_dispatch(EdgeListGraph& g, Size N, 
                          WeightMap weight, DistanceMap distance, 
                          const bgl_named_params<P, T, R>& params)
    {
      dummy_property_map dummy_pred;
      return 
        detail::bellman_dispatch2
          (g, 
           get_param(params, root_vertex_t()),
           N, weight,
           choose_param(get_param(params, vertex_predecessor), dummy_pred),
           distance,
           params);
    }
  } // namespace detail

  template <class EdgeListGraph, class Size, class P, class T, class R>
  bool bellman_ford_shortest_paths
    (EdgeListGraph& g, Size N, 
     const bgl_named_params<P, T, R>& params)
  {                                
    return detail::bellman_dispatch
      (g, N,
       choose_const_pmap(get_param(params, edge_weight), g, edge_weight),
       choose_pmap(get_param(params, vertex_distance), g, vertex_distance),
       params);
  }

  template <class EdgeListGraph, class Size>
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  inline typename boost::disable_if<
    parameter::is_argument_pack<Size>,
#endif
    bool
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  >::type
#endif
  bellman_ford_shortest_paths(EdgeListGraph& g, Size N)
  {
    bgl_named_params<int,int> params(0);
    return bellman_ford_shortest_paths(g, N, params);
  }

  template <class VertexAndEdgeListGraph, class P, class T, class R>
  bool bellman_ford_shortest_paths
    (VertexAndEdgeListGraph& g, 
     const bgl_named_params<P, T, R>& params)
  {               
    BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<VertexAndEdgeListGraph> ));
    return detail::bellman_dispatch
      (g, num_vertices(g),
       choose_const_pmap(get_param(params, edge_weight), g, edge_weight),
       choose_pmap(get_param(params, vertex_distance), g, vertex_distance),
       params);
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <typename EdgeListGraph, typename Args>
  inline typename boost::enable_if<
    parameter::is_argument_pack<Args>, bool
  >::type
  bellman_ford_shortest_paths(EdgeListGraph& g, const Args& args)
  {
    using namespace boost::graph::keywords;
    typedef typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::weight_map,
        edge_weight_t,
        EdgeListGraph
    >::type weight_map_type;
    weight_map_type e_w_map = detail::override_const_property(
        args,
        _weight_map,
        g,
        edge_weight
    );
    typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::distance_map,
        vertex_distance_t,
        EdgeListGraph
    >::type v_d_map = detail::override_const_property(
        args,
        _distance_map,
        g,
        vertex_distance
    );
    detail::bellman_initialize_distance_map(
      g,
      e_w_map,
      v_d_map,
      args,
      typename mpl::has_key<
        Args,
        boost::graph::keywords::tag::root_vertex
      >::type()
    );
    typedef typename boost::property_traits<weight_map_type>::value_type D;
    dummy_property_map dummy_prop;
    typename boost::parameter::binding<
        Args, 
        boost::graph::keywords::tag::predecessor_map,
        dummy_property_map&
    >::type pred_map = args[_predecessor_map | dummy_prop];
    return bellman_ford_shortest_paths(
      g,
      args[_size || detail::make_vertex_count_nullary_function(g)],
      e_w_map,
      pred_map,
      v_d_map,
      args[_distance_combine | closed_plus<D>()],
      args[_distance_compare | std::less<D>()],
      args[_visitor | bellman_visitor<>()]
    );
  }

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
  template <typename Graph, typename TA \
            BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA)> \
  inline typename boost::enable_if< \
    parameter::are_tagged_arguments< \
      TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
    >, bool \
  >::type name( \
    const Graph& g, const TA& ta \
    BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta) \
  ) \
  { \
    return name(g, parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta))); \
  }

BOOST_PP_REPEAT_FROM_TO(1, 9, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, bellman_ford_shortest_paths)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

} // namespace boost

#endif // BOOST_GRAPH_BELLMAN_FORD_SHORTEST_PATHS_HPP
