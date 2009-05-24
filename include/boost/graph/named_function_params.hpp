//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP
#define BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP

#include <boost/graph/properties.hpp>
#include <boost/ref.hpp>

namespace boost {

  struct distance_compare_t { };
  struct distance_combine_t { };
  struct distance_inf_t { };
  struct distance_zero_t { };
  struct buffer_param_t { };
  struct edge_copy_t { };
  struct vertex_copy_t { };
  struct vertex_isomorphism_t { };
  struct vertex_invariant_t { };
  struct vertex_invariant1_t { };
  struct vertex_invariant2_t { };
  struct edge_compare_t { };
  struct vertex_max_invariant_t { };
  struct orig_to_copy_t { };
  struct root_vertex_t { };
  struct polling_t { };
  struct lookahead_t { };
  struct in_parallel_t { };
  struct attractive_force_t { };
  struct repulsive_force_t { };
  struct force_pairs_t { };
  struct cooling_t { };
  struct vertex_displacement_t { };
  struct iterations_t { };
  struct diameter_range_t { };
  struct learning_constant_range_t { };

#define BOOST_BGL_DECLARE_NAMED_PARAMS \
    BOOST_BGL_ONE_PARAM_CREF(weight_map, edge_weight) \
    BOOST_BGL_ONE_PARAM_CREF(weight_map2, edge_weight2) \
    BOOST_BGL_ONE_PARAM_CREF(distance_map, vertex_distance) \
    BOOST_BGL_ONE_PARAM_CREF(predecessor_map, vertex_predecessor) \
    BOOST_BGL_ONE_PARAM_CREF(rank_map, vertex_rank) \
    BOOST_BGL_ONE_PARAM_CREF(root_map, vertex_root) \
    BOOST_BGL_ONE_PARAM_CREF(root_vertex, root_vertex) \
    BOOST_BGL_ONE_PARAM_CREF(edge_centrality_map, edge_centrality) \
    BOOST_BGL_ONE_PARAM_CREF(centrality_map, vertex_centrality) \
    BOOST_BGL_ONE_PARAM_CREF(color_map, vertex_color) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_color_map, vertex_color) \
    BOOST_BGL_ONE_PARAM_CREF(edge_color_map, edge_color) \
    BOOST_BGL_ONE_PARAM_CREF(capacity_map, edge_capacity) \
    BOOST_BGL_ONE_PARAM_CREF(residual_capacity_map, edge_residual_capacity) \
    BOOST_BGL_ONE_PARAM_CREF(reverse_edge_map, edge_reverse) \
    BOOST_BGL_ONE_PARAM_CREF(discover_time_map, vertex_discover_time) \
    BOOST_BGL_ONE_PARAM_CREF(lowpoint_map, vertex_lowpoint) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_index_map, vertex_index) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_index1_map, vertex_index1) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_index2_map, vertex_index2) \
    BOOST_BGL_ONE_PARAM_CREF(visitor, graph_visitor) \
    BOOST_BGL_ONE_PARAM_CREF(distance_compare, distance_compare) \
    BOOST_BGL_ONE_PARAM_CREF(distance_combine, distance_combine) \
    BOOST_BGL_ONE_PARAM_CREF(distance_inf, distance_inf) \
    BOOST_BGL_ONE_PARAM_CREF(distance_zero, distance_zero) \
    BOOST_BGL_ONE_PARAM_CREF(edge_copy, edge_copy) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_copy, vertex_copy) \
    BOOST_BGL_ONE_PARAM_REF(buffer, buffer_param) \
    BOOST_BGL_ONE_PARAM_CREF(orig_to_copy, orig_to_copy) \
    BOOST_BGL_ONE_PARAM_CREF(isomorphism_map, vertex_isomorphism) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_invariant, vertex_invariant) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_invariant1, vertex_invariant1) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_invariant2, vertex_invariant2) \
    BOOST_BGL_ONE_PARAM_CREF(vertex_max_invariant, vertex_max_invariant) \
    BOOST_BGL_ONE_PARAM_CREF(polling, polling) \
    BOOST_BGL_ONE_PARAM_CREF(lookahead, lookahead) \
    BOOST_BGL_ONE_PARAM_CREF(in_parallel, in_parallel) \
    BOOST_BGL_ONE_PARAM_CREF(displacement_map, vertex_displacement) \
    BOOST_BGL_ONE_PARAM_CREF(attractive_force, attractive_force) \
    BOOST_BGL_ONE_PARAM_CREF(repulsive_force, repulsive_force) \
    BOOST_BGL_ONE_PARAM_CREF(force_pairs, force_pairs) \
    BOOST_BGL_ONE_PARAM_CREF(cooling, cooling) \
    BOOST_BGL_ONE_PARAM_CREF(iterations, iterations) \
    BOOST_BGL_ONE_PARAM_CREF(diameter_range, diameter_range) \
    BOOST_BGL_ONE_PARAM_CREF(learning_constant_range, learning_constant_range)

  template <typename T, typename Tag, typename Base = no_property>
  struct bgl_named_params : public Base
  {
    typedef bgl_named_params self;
    typedef Base next_type;
    typedef Tag tag_type;
    typedef T value_type;
    bgl_named_params(T v = T()) : m_value(v) { }
    bgl_named_params(T v, const Base& b) : Base(b), m_value(v) { }
    T m_value;

#define BOOST_BGL_ONE_PARAM_REF(name, key) \
    template <typename PType> \
    bgl_named_params<boost::reference_wrapper<PType>, BOOST_PP_CAT(key, _t), self> \
    name(PType& p) const { \
      typedef bgl_named_params<boost::reference_wrapper<PType>, BOOST_PP_CAT(key, _t), self> Params; \
      return Params(boost::ref(p), *this); \
    } \

#define BOOST_BGL_ONE_PARAM_CREF(name, key) \
    template <typename PType> \
    bgl_named_params<PType, BOOST_PP_CAT(key, _t), self> \
    name(const PType& p) const { \
      typedef bgl_named_params<PType, BOOST_PP_CAT(key, _t), self> Params; \
      return Params(p, *this); \
    } \

BOOST_BGL_DECLARE_NAMED_PARAMS

#undef BOOST_BGL_ONE_PARAM_REF
#undef BOOST_BGL_ONE_PARAM_CREF
  };

#define BOOST_BGL_ONE_PARAM_REF(name, key) \
    template <typename PType> \
    bgl_named_params<boost::reference_wrapper<PType>, BOOST_PP_CAT(key, _t)> \
    name(PType& p) { \
      typedef bgl_named_params<boost::reference_wrapper<PType>, BOOST_PP_CAT(key, _t)> Params; \
      return Params(boost::ref(p)); \
    } \

#define BOOST_BGL_ONE_PARAM_CREF(name, key) \
    template <typename PType> \
    bgl_named_params<PType, BOOST_PP_CAT(key, _t)> \
    name(const PType& p) { \
      typedef bgl_named_params<PType, BOOST_PP_CAT(key, _t)> Params; \
      return Params(p); \
    } \

BOOST_BGL_DECLARE_NAMED_PARAMS

#undef BOOST_BGL_ONE_PARAM_REF
#undef BOOST_BGL_ONE_PARAM_CREF

  namespace detail {
    struct unused_tag_type {};
  }
  typedef bgl_named_params<char, detail::unused_tag_type> no_named_parameters;

  //===========================================================================
  // Functions for extracting parameters from bgl_named_params

  template <class Tag1, class Tag2, class T1, class Base>
  inline
  typename property_value< bgl_named_params<T1,Tag1,Base>, Tag2>::type
  get_param(const bgl_named_params<T1,Tag1,Base>& p, Tag2 tag2)
  {
    enum { match = detail::same_property<Tag1,Tag2>::value };
    typedef typename
      property_value< bgl_named_params<T1,Tag1,Base>, Tag2>::type T2;
    T2* t2 = 0;
    typedef detail::property_value_dispatch<match> Dispatcher;
    return Dispatcher::const_get_value(p, t2, tag2);
  }


  namespace detail {
    // MSVC++ workaround
    template <class Param>
    struct choose_param_helper {
      template <class Default> struct result { typedef Param type; };
      template <typename Default>
      static const Param& apply(const Param& p, const Default&) { return p; }
    };
    template <>
    struct choose_param_helper<error_property_not_found> {
      template <class Default> struct result { typedef Default type; };
      template <typename Default>
      static const Default& apply(const error_property_not_found&, const Default& d)
        { return d; }
    };
  } // namespace detail

  template <class P, class Default> 
  const typename detail::choose_param_helper<P>::template result<Default>::type&
  choose_param(const P& param, const Default& d) { 
    return detail::choose_param_helper<P>::apply(param, d);
  }

  template <typename T>
  inline bool is_default_param(const T&) { return false; }

  inline bool is_default_param(const detail::error_property_not_found&)
    { return true; }

  namespace detail {

    struct choose_parameter {
      template <class P, class Graph, class Tag>
      struct bind_ {
        typedef const P& const_result_type;
        typedef const P& result_type;
        typedef P type;
      };

      template <class P, class Graph, class Tag>
      static typename bind_<P, Graph, Tag>::const_result_type
      const_apply(const P& p, const Graph&, Tag&) 
      { return p; }

      template <class P, class Graph, class Tag>
      static typename bind_<P, Graph, Tag>::result_type
      apply(const P& p, Graph&, Tag&) 
      { return p; }
    };

    struct choose_default_param {
      template <class P, class Graph, class Tag>
      struct bind_ {
        typedef typename property_map<Graph, Tag>::type 
          result_type;
        typedef typename property_map<Graph, Tag>::const_type 
          const_result_type;
        typedef typename property_map<Graph, Tag>::const_type 
          type;
      };

      template <class P, class Graph, class Tag>
      static typename bind_<P, Graph, Tag>::const_result_type
      const_apply(const P&, const Graph& g, Tag tag) { 
        return get(tag, g); 
      }
      template <class P, class Graph, class Tag>
      static typename bind_<P, Graph, Tag>::result_type
      apply(const P&, Graph& g, Tag tag) { 
        return get(tag, g); 
      }
    };

    template <class Param>
    struct choose_property_map {
      typedef choose_parameter type;
    };
    template <>
    struct choose_property_map<detail::error_property_not_found> {
      typedef choose_default_param type;
    };

    template <class Param, class Graph, class Tag>
    struct choose_pmap_helper {
      typedef typename choose_property_map<Param>::type Selector;
      typedef typename Selector:: template bind_<Param, Graph, Tag> Bind;
      typedef Bind type;
      typedef typename Bind::result_type result_type;
      typedef typename Bind::const_result_type const_result_type;
      typedef typename Bind::type result;
    };

    // used in the max-flow algorithms
    template <class Graph, class P, class T, class R>
    struct edge_capacity_value
    {
      typedef bgl_named_params<P, T, R> Params;
      typedef typename property_value< Params, edge_capacity_t>::type Param;
      typedef typename detail::choose_pmap_helper<Param, Graph,
        edge_capacity_t>::result CapacityEdgeMap;
      typedef typename property_traits<CapacityEdgeMap>::value_type type;
    };

  } // namespace detail
  

  // Use this function instead of choose_param() when you want
  // to avoid requiring get(tag, g) when it is not used. 
  template <typename Param, typename Graph, typename PropertyTag>
  typename
    detail::choose_pmap_helper<Param,Graph,PropertyTag>::const_result_type
  choose_const_pmap(const Param& p, const Graph& g, PropertyTag tag)
  { 
    typedef typename 
      detail::choose_pmap_helper<Param,Graph,PropertyTag>::Selector Choice;
    return Choice::const_apply(p, g, tag);
  }

  template <typename Param, typename Graph, typename PropertyTag>
  typename detail::choose_pmap_helper<Param,Graph,PropertyTag>::result_type
  choose_pmap(const Param& p, Graph& g, PropertyTag tag)
  { 
    typedef typename 
      detail::choose_pmap_helper<Param,Graph,PropertyTag>::Selector Choice;
    return Choice::apply(p, g, tag);
  }

} // namespace boost

#undef BOOST_BGL_DECLARE_NAMED_PARAMS

#endif // BOOST_GRAPH_NAMED_FUNCTION_PARAMS_HPP
