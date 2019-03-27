//============================================================================
// Copyright 2013 University of Warsaw.
// Authors: Piotr Wygocki
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//============================================================================
//
// This algorithm is described in
// "Network Flows: Theory, Algorithms, and Applications"
// by Ahuja, Magnanti, Orlin.

#ifndef BOOST_GRAPH_SUCCESSIVE_SHORTEST_PATH_HPP
#define BOOST_GRAPH_SUCCESSIVE_SHORTEST_PATH_HPP

#include <numeric>

#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/detail/augment.hpp>

namespace boost { namespace detail {

template <class Graph, class Weight, class Distance, class Reversed>
class MapReducedWeight :
    public put_get_helper<typename property_traits<Weight>::value_type, MapReducedWeight<Graph, Weight, Distance, Reversed> > {
    typedef graph_traits<Graph> gtraits;
public:
    typedef boost::readable_property_map_tag category;
    typedef typename property_traits<Weight>::value_type value_type;
    typedef value_type reference;
    typedef typename gtraits::edge_descriptor key_type;
    MapReducedWeight(const Graph & g, Weight w, Distance d, Reversed r) :
        g_(g), weight_(w), distance_(d), rev_(r) {}

    reference operator[](key_type v) const {
        return get(distance_, source(v, g_)) - get(distance_,target(v, g_)) + get(weight_, v);
    }
private:
    const Graph & g_;
    Weight weight_;
    Distance distance_;
    Reversed rev_;
};

template <class Graph, class Weight, class Distance, class Reversed>
MapReducedWeight<Graph, Weight, Distance, Reversed>
make_mapReducedWeight(const Graph & g, Weight w, Distance d, Reversed r)  {
    return MapReducedWeight<Graph, Weight, Distance, Reversed>(g, w, d, r);
}

}} // namespace boost::detail

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/core/enable_if.hpp>
#endif

namespace boost {

template <class Graph, class Capacity, class ResidualCapacity, class Reversed, class Pred, class Weight, class Distance, class Distance2, class VertexIndex>
void successive_shortest_path_nonnegative_weights(
        const Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        Distance distance,
        Distance2 distance_prev
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<
                Capacity, ResidualCapacity, Weight, Reversed, VertexIndex,
                Pred, Distance, Distance2
            >,
            mpl::true_
        >::type = mpl::true_()
#endif
        )
{
    filtered_graph<const Graph, is_residual_edge<ResidualCapacity> >
        gres = detail::residual_graph(g, residual_capacity);
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;

    BGL_FORALL_EDGES_T(e, g, Graph) {
        put(residual_capacity, e, get(capacity, e));
    }

    BGL_FORALL_VERTICES_T(v, g, Graph) {
        put(distance_prev, v, 0);
    }

    while(true) {
        BGL_FORALL_VERTICES_T(v, g, Graph) {
            put(pred, v, edge_descriptor());
        }
        dijkstra_shortest_paths(gres, s,
                weight_map(detail::make_mapReducedWeight(gres, weight, distance_prev, rev)).
                distance_map(distance).
                vertex_index_map(index).
                visitor(make_dijkstra_visitor(record_edge_predecessors(pred, on_edge_relaxed()))));

        if(get(pred, t) == edge_descriptor()) {
            break;
        }

        BGL_FORALL_VERTICES_T(v, g, Graph) {
            put(distance_prev, v, get(distance_prev, v) + get(distance, v));
        }

        detail::augment(g, s, t, pred, residual_capacity, rev);
    }
}

} // namespace boost

//in this namespace argument dispatching takes place
namespace boost { namespace detail {

template <class Graph, class Capacity, class ResidualCapacity, class Weight, class Reversed, class Pred, class Distance, class Distance2, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch3(
        const Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        Distance dist,
        Distance2 dist_pred) {
    successive_shortest_path_nonnegative_weights(g, s, t, capacity, residual_capacity, weight, rev, index, pred, dist, dist_pred);
}

//setting default distance map
template <class Graph, class Capacity, class ResidualCapacity, class Weight, class Reversed, class Pred, class Distance, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch3(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        Distance dist,
        param_not_found) {
    typedef typename property_traits<Weight>::value_type D;

    std::vector<D> d_map(num_vertices(g));

    successive_shortest_path_nonnegative_weights(g, s, t, capacity, residual_capacity, weight, rev, index, pred, dist,
                             make_iterator_property_map(d_map.begin(), index));
}

template <class Graph, class P, class T, class R, class Capacity, class ResidualCapacity, class Weight, class Reversed, class Pred, class Distance, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch2(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        Distance dist,
        const bgl_named_params<P, T, R>& params) {
    successive_shortest_path_nonnegative_weights_dispatch3(g, s, t, capacity, residual_capacity, weight, rev, index, pred, dist, get_param(params, vertex_distance2));
}

//setting default distance map
template <class Graph, class P, class T, class R, class Capacity, class ResidualCapacity, class Weight, class Reversed, class Pred, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch2(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        param_not_found,
        const bgl_named_params<P, T, R>& params) {
    typedef typename property_traits<Weight>::value_type D;

    std::vector<D> d_map(num_vertices(g));

    successive_shortest_path_nonnegative_weights_dispatch3(g, s, t, capacity, residual_capacity, weight, rev, index, pred,
            make_iterator_property_map(d_map.begin(), index),
            get_param(params, vertex_distance2));
}

template <class Graph, class P, class T, class R, class Capacity, class ResidualCapacity, class Weight, class Reversed, class Pred, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch1(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        Pred pred,
        const bgl_named_params<P, T, R>& params) {
    successive_shortest_path_nonnegative_weights_dispatch2(g, s, t, capacity, residual_capacity, weight,  rev, index, pred,
                                get_param(params, vertex_distance), params);
}

//setting default predecessors map
template <class Graph, class P, class T, class R, class Capacity, class ResidualCapacity, class Weight, class Reversed, class VertexIndex>
void successive_shortest_path_nonnegative_weights_dispatch1(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        Capacity capacity,
        ResidualCapacity residual_capacity,
        Weight weight,
        Reversed rev,
        VertexIndex index,
        param_not_found,
        const bgl_named_params<P, T, R>& params) {
    typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;
    std::vector<edge_descriptor> pred_vec(num_vertices(g));

    successive_shortest_path_nonnegative_weights_dispatch2(g, s, t, capacity, residual_capacity, weight, rev, index,
            make_iterator_property_map(pred_vec.begin(), index),
            get_param(params, vertex_distance), params);
}

}} // namespace boost::detail

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>

namespace boost {

template <typename Graph, typename Args>
void successive_shortest_path_nonnegative_weights(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        const Args& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()) {
    typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::vertex_index_map,
        vertex_index_t,
        Graph
    >::type v_i_map = detail::override_const_property(
        args,
        boost::graph::keywords::_vertex_index_map,
        g,
        vertex_index
    );
    typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::capacity_map,
        edge_capacity_t,
        Graph
    >::type e_c_map = detail::override_const_property(
        args,
        boost::graph::keywords::_capacity_map,
        g,
        edge_capacity
    );
    typename boost::detail::override_property_result<
        Args,
        boost::graph::keywords::tag::residual_capacity_map,
        edge_residual_capacity_t,
        Graph
    >::type e_rc_map = detail::override_property(
        args,
        boost::graph::keywords::_residual_capacity_map,
        g,
        edge_residual_capacity
    );
    typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::reverse_edge_map,
        edge_reverse_t,
        Graph
    >::type e_rev_map = detail::override_const_property(
        args,
        boost::graph::keywords::_reverse_edge_map,
        g,
        edge_reverse
    );
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    const Edge zer_edge = Edge();
    boost::detail::make_property_map_from_arg_pack_gen<
        boost::graph::keywords::tag::predecessor_map,
        Edge
    > pred_map_gen(zer_edge);
    typedef typename boost::detail::override_const_property_result<
        Args,
        boost::graph::keywords::tag::weight_map,
        edge_weight_t,
        Graph
    >::type WeightMap;
    WeightMap e_w_map = detail::override_const_property(
        args,
        boost::graph::keywords::_weight_map,
        g,
        edge_weight
    );
    typedef typename boost::property_traits<WeightMap>::value_type Distance;
    const Distance zero_distance = Distance();
    boost::detail::make_property_map_from_arg_pack_gen<
        boost::graph::keywords::tag::distance_map,
        Distance
    > dist_map_gen(zero_distance);
    boost::detail::make_property_map_from_arg_pack_gen<
        boost::graph::keywords::tag::distance_map2,
        Distance
    > dist_map2_gen(zero_distance);
    successive_shortest_path_nonnegative_weights(
        g, s, t,
        e_c_map,
        e_rc_map,
        e_w_map,
        e_rev_map,
        v_i_map,
        pred_map_gen(g, args),
        dist_map_gen(g, args),
        dist_map2_gen(g, args)
    );
}

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
    template < \
        typename Graph, typename TA \
        BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA) \
    > \
    inline void name( \
        Graph& g, \
        typename graph_traits<Graph>::vertex_descriptor s, \
        typename graph_traits<Graph>::vertex_descriptor t, \
        const TA& ta \
        BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
        typename boost::enable_if< \
            parameter::are_tagged_arguments< \
                TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
            >, mpl::true_ \
        >::type = mpl::true_() \
    ) \
    { \
        name( \
            g, s, t, \
            parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta)) \
        ); \
    }

BOOST_PP_REPEAT_FROM_TO(
    1, 9, BOOST_GRAPH_PP_FUNCTION_OVERLOAD,
    successive_shortest_path_nonnegative_weights
)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD

} // namespace boost
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

namespace boost {

template <class Graph, class P, class T, class R>
inline void successive_shortest_path_nonnegative_weights(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t,
        const bgl_named_params<P, T, R>& params) {

    return detail::successive_shortest_path_nonnegative_weights_dispatch1(g, s, t,
           choose_const_pmap(get_param(params, edge_capacity), g, edge_capacity),
           choose_pmap(get_param(params, edge_residual_capacity),
                       g, edge_residual_capacity),
           choose_const_pmap(get_param(params, edge_weight), g, edge_weight),
           choose_const_pmap(get_param(params, edge_reverse), g, edge_reverse),
           choose_const_pmap(get_param(params, vertex_index), g, vertex_index),
           get_param(params, vertex_predecessor),
           params);
}

template <class Graph>
inline void successive_shortest_path_nonnegative_weights(
        Graph &g,
        typename graph_traits<Graph>::vertex_descriptor s,
        typename graph_traits<Graph>::vertex_descriptor t) {
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    successive_shortest_path_nonnegative_weights(
        g, s, t, parameter::compose()
    );
#else
    bgl_named_params<int, buffer_param_t> params(0);
    successive_shortest_path_nonnegative_weights(g, s, t, params);
#endif
}


}//boost
#endif /* BOOST_GRAPH_SUCCESSIVE_SHORTEST_PATH_HPP */
