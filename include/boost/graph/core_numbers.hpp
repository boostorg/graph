//
//=======================================================================
// Copyright 2007 Stanford University
// Authors: David Gleich
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_CORE_NUMBERS_HPP
#define BOOST_GRAPH_CORE_NUMBERS_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/parameter/config.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/mpl/vector.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/declval.hpp>

#if defined(BOOST_NO_CXX11_DECLTYPE)
#include <boost/typeof/typeof.hpp>
#endif

namespace boost { namespace detail {

#if !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
    template <typename T, typename G>
    class is_core_numbers_visitor_impl
    {
        typedef typename boost::remove_const<T>::type _m_T;

        template <typename B, typename P>
        static graph_yes_tag
            _check_exam_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().examine_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().examine_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_exam_v(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_exam_e(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().examine_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().examine_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_exam_e(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_end_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().finish_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().finish_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_end_v(...);

    public:
        typedef mpl::bool_<
            (
                sizeof(
                    is_core_numbers_visitor_impl<T,G>::_check_exam_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_core_numbers_visitor_impl<T,G>::_check_exam_e(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_core_numbers_visitor_impl<T,G>::_check_end_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            )
        > type;
    };

    template <typename T, typename G>
    struct is_core_numbers_visitor
      : mpl::eval_if<
        is_bgl_graph<G>,
        is_core_numbers_visitor_impl<T,G>,
        mpl::false_
      >::type
    { };

    typedef argument_with_graph_predicate<
      is_core_numbers_visitor
    > core_numbers_visitor_predicate;
#else   // defined(BOOST_NO_CXX11_DECLTYPE) && !defined(BOOST_TYPEOF_KEYWORD)
    typedef visitor_predicate core_numbers_visitor_predicate;
#endif  // !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
}}
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS

#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/iterator/reverse_iterator.hpp>
#include <boost/concept/assert.hpp>

/*
 * core_numbers
 *
 * Requirement: IncidenceGraph
 */

// History
//
// 30 July 2007
// Added visitors to the implementation
//
// 8 February 2008
// Fixed headers and missing typename

namespace boost {

    // A linear time O(m) algorithm to compute the indegree core number
    // of a graph for unweighted graphs.
    //
    // and a O((n+m) log n) algorithm to compute the in-edge-weight core
    // numbers of a weighted graph.
    //
    // The linear algorithm comes from:
    // Vladimir Batagelj and Matjaz Zaversnik, "An O(m) Algorithm for Cores
    // Decomposition of Networks."  Sept. 1 2002.

    template <typename Visitor, typename Graph>
    struct CoreNumbersVisitorConcept {
        void constraints()
        {
            BOOST_CONCEPT_ASSERT(( CopyConstructibleConcept<Visitor> ));
            vis.examine_vertex(u,g);
            vis.finish_vertex(u,g);
            vis.examine_edge(e,g);
        }
        Visitor vis;
        Graph g;
        typename graph_traits<Graph>::vertex_descriptor u;
        typename graph_traits<Graph>::edge_descriptor e;
    };

    template <class Visitors = null_visitor>
    class core_numbers_visitor : public bfs_visitor<Visitors> {
        public:
        core_numbers_visitor() {}
        core_numbers_visitor(Visitors vis)
            : bfs_visitor<Visitors>(vis) {}

        private:
        template <class Vertex, class Graph>
        void initialize_vertex(Vertex, Graph&) {}

        template <class Vertex, class Graph>
        void discover_vertex(Vertex , Graph&) {}

        template <class Vertex, class Graph>
        void gray_target(Vertex, Graph&) {}

        template <class Vertex, class Graph>
        void black_target(Vertex, Graph&) {}

        template <class Edge, class Graph>
        void tree_edge(Edge, Graph&) {}

        template <class Edge, class Graph>
        void non_tree_edge(Edge, Graph&) {}
    };

    template <class Visitors>
    core_numbers_visitor<Visitors> make_core_numbers_visitor(Visitors vis)
    { return core_numbers_visitor<Visitors>(vis); }

    typedef core_numbers_visitor<> default_core_numbers_visitor;
}

#include <boost/property_map/property_map.hpp>

namespace boost { namespace detail {

    // The core numbers start as the indegree or inweight.
    // This function will initialize these values.
    template <typename Graph, typename CoreMap, typename EdgeWeightMap>
    void compute_in_degree_map(Graph& g, CoreMap d, EdgeWeightMap wm)
    {
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;
        typename graph_traits<Graph>::out_edge_iterator ei,ei_end;

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            put(d,*vi,0);
        }

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            for (boost::tie(ei,ei_end) = out_edges(*vi,g); ei!=ei_end; ++ei)
            {
                put(d,target(*ei,g),get(d,target(*ei,g))+get(wm,*ei));
            }
        }
    }

    // the version for weighted graphs is a little different
    template <
        typename Graph, typename CoreMap, typename EdgeWeightMap,
        typename MutableQueue, typename Visitor
    >
    typename property_traits<CoreMap>::value_type
    core_numbers_impl(
        Graph& g, CoreMap c, EdgeWeightMap wm, MutableQueue& Q, Visitor vis
    )
    {
        typename property_traits<CoreMap>::value_type v_cn = 0;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;

        while (!Q.empty())
        {
            // remove v from the Q, and then decrease the core numbers
            // of its successors
            vertex v = Q.top();
            vis.examine_vertex(v,g);
            Q.pop();
            v_cn = get(c,v);
            typename graph_traits<Graph>::out_edge_iterator oi,oi_end;

            for (boost::tie(oi,oi_end) = out_edges(v,g); oi!=oi_end; ++oi)
            {
                vis.examine_edge(*oi,g);
                vertex u = target(*oi,g);

                // if c[u] > c[v], then u is still in the graph,
                if (get(c,u) > v_cn)
                {
                    // remove the edge
                    put(c,u,get(c,u)-get(wm,*oi));
                    if (Q.contains(u)) Q.update(u);
                }
            }

            vis.finish_vertex(v,g);
        }

        return (v_cn);
    }

    template <
        typename Graph, typename CoreMap, typename EdgeWeightMap,
        typename IndexMap, typename CoreNumVisitor
    >
    typename property_traits<CoreMap>::value_type
    core_numbers_dispatch(
        Graph& g, CoreMap c, EdgeWeightMap wm, IndexMap im,
        CoreNumVisitor vis, mpl::true_
    )
    {
        detail::compute_in_degree_map(g, c, wm);
        typedef typename property_traits<CoreMap>::value_type D;
        typedef std::less<D> Cmp;
        // build the mutable queue
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;
        std::vector<std::size_t> index_in_heap_data(num_vertices(g));
        typedef iterator_property_map<
            std::vector<std::size_t>::iterator, IndexMap
        > index_in_heap_map_type;
        index_in_heap_map_type index_in_heap_map(
            index_in_heap_data.begin(), im
        );
        d_ary_heap_indirect<
            vertex, 4, index_in_heap_map_type, CoreMap, Cmp
        > Q(c, index_in_heap_map, Cmp());
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            Q.push(*vi);
        }

        return core_numbers_impl(g, c, wm, Q, vis);
    }

    // the version for the unweighted case
    // for this functions CoreMap must be initialized
    // with the in degree of each vertex
    template <
        typename Graph, typename CoreMap, typename PositionMap,
        typename Visitor
    >
    typename property_traits<CoreMap>::value_type
    core_numbers_impl(Graph& g, CoreMap c, PositionMap pos, Visitor vis)
    {
        typedef typename graph_traits<Graph>::vertices_size_type size_type;
        typedef typename graph_traits<Graph>::degree_size_type degree_type;
        typedef typename graph_traits<Graph>::vertex_descriptor vertex;
        typename graph_traits<Graph>::vertex_iterator vi,vi_end;

        // store the vertex core numbers
        typename property_traits<CoreMap>::value_type v_cn = 0;

        // compute the maximum degree (degrees are in the coremap)
        typename graph_traits<Graph>::degree_size_type max_deg = 0;

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            max_deg = (
                std::max<typename graph_traits<Graph>::degree_size_type>
            )(max_deg, get(c,*vi));
        }

        // store the vertices in bins by their degree
        // allocate two extra locations to ease boundary cases
        std::vector<size_type> bin(max_deg+2);

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            ++bin[get(c,*vi)];
        }

        // this loop sets bin[d] to the starting position of vertices
        // with degree d in the vert array for the bucket sort
        size_type cur_pos = 0;

        for (degree_type cur_deg = 0; cur_deg < max_deg+2; ++cur_deg)
        {
            degree_type tmp = bin[cur_deg];
            bin[cur_deg] = cur_pos;
            cur_pos += tmp;
        }

        // perform the bucket sort with pos and vert so that
        // pos[0] is the vertex of smallest degree
        std::vector<vertex> vert(num_vertices(g));

        for (boost::tie(vi,vi_end) = vertices(g); vi!=vi_end; ++vi)
        {
            vertex v=*vi;
            size_type p=bin[get(c,v)];
            put(pos,v,p);
            vert[p]=v;
            ++bin[get(c,v)];
        }

        // we ``abused'' bin while placing the vertices, now,
        // we need to restore it
        std::copy(
            boost::make_reverse_iterator(bin.end()-2),
            boost::make_reverse_iterator(bin.begin()),
            boost::make_reverse_iterator(bin.end()-1)
        );

        // now simulate removing the vertices
        for (size_type i=0; i < num_vertices(g); ++i)
        {
            vertex v = vert[i];
            vis.examine_vertex(v,g);
            v_cn = get(c,v);
            typename graph_traits<Graph>::out_edge_iterator oi,oi_end;

            for (boost::tie(oi,oi_end) = out_edges(v,g); oi!=oi_end; ++oi)
            {
                vis.examine_edge(*oi,g);
                vertex u = target(*oi,g);

                // if c[u] > c[v], then u is still in the graph,
                if (get(c,u) > v_cn)
                {
                    degree_type deg_u = get(c,u);
                    degree_type pos_u = get(pos,u);
                    // w is the first vertex with the same degree as u
                    // (this is the resort operation!)
                    degree_type pos_w = bin[deg_u];
                    vertex w = vert[pos_w];

                    if (u!=v)
                    {
                        // swap u and w
                        put(pos,u,pos_w);
                        put(pos,w,pos_u);
                        vert[pos_w] = u;
                        vert[pos_u] = w;
                    }

                    // now, the vertices array is sorted assuming
                    // we perform the following step
                    // start the set of vertices with degree of u
                    // one into the future (this now points at
                    // vertex w which we swapped with u).
                    ++bin[deg_u];
                    // we are removing v from the graph,
                    // so u's degree decreases
                    put(c,u,get(c,u)-1);
                }
            }

            vis.finish_vertex(v,g);
        }

        return v_cn;
    }

    template <
        typename Graph, typename CoreMap,
        typename IndexMap, typename CoreNumVisitor
    >
    inline typename property_traits<CoreMap>::value_type
    core_numbers_dispatch(
        Graph& g, CoreMap c, dummy_property_map, IndexMap im,
        CoreNumVisitor vis, mpl::false_
    )
    {
        detail::compute_in_degree_map(
            g,
            c,
            static_property_map<
                typename property_traits<CoreMap>::value_type
            >(1)
        );
        typedef typename graph_traits<Graph>::vertices_size_type size_type;
        std::vector<size_type> p(num_vertices(g));
        return core_numbers_impl(
            g,
            c,
            make_iterator_property_map(p.begin(), im),
            vis
        );
    }
}} // namespace boost::detail

#include <boost/core/enable_if.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/graph/detail/traits.hpp>
#include <boost/parameter/preprocessor.hpp>
#include <boost/mpl/has_key.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#elif defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/parameter/value_type.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif

namespace boost {

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    BOOST_PARAMETER_FUNCTION(
        (
            boost::lazy_enable_if<
                typename mpl::has_key<
                    Args,
                    boost::graph::keywords::tag::result
                >::type,
                detail::property_map_value<
                    Args,
                    boost::graph::keywords::tag::result
                >
            >
        ), core_numbers, ::boost::graph::keywords::tag,
        (required
            (graph, *(detail::argument_predicate<is_bgl_graph>))
            (result
              , *(
                    detail::argument_with_graph_predicate<
                        detail::is_vertex_property_map_of_graph
                    >
                )
            )
        )
        (deduced
            (optional
                (weight_map
                  , *(
                        detail::argument_with_graph_predicate<
                            detail::is_edge_property_map_of_graph
                        >
                    )
                  , detail::edge_or_dummy_property_map(graph, edge_weight)
                )
                (vertex_index_map
                  , *(
                        detail::argument_with_graph_predicate<
                            detail::is_vertex_to_integer_map_of_graph
                        >
                    )
                  , detail::vertex_or_dummy_property_map(graph, vertex_index)
                )
                (visitor
                  , *(detail::core_numbers_visitor_predicate)
                  , default_core_numbers_visitor()
                )
            )
        )
    )
    {
        typename boost::remove_const<
            typename boost::remove_reference<visitor_type>::type
        >::type vis = visitor;
        return detail::core_numbers_dispatch(
            graph,
            result,
            weight_map,
            vertex_index_map,
            vis,
            typename mpl::eval_if<
                boost::is_same<
                    dummy_property_map,
                    typename boost::remove_const<
                        typename boost::remove_reference<
                            weight_map_type
                        >::type
                    >::type
                >,
                mpl::false_,
                mpl::true_
            >::type()
        );
    }
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    // non-named parameter version for the weighted case
    template <
        typename Graph, typename CoreMap, typename EdgeWeightMap,
        typename VertexIndexMap, typename CoreNumVisitor
    >
    inline typename property_traits<CoreMap>::value_type
    core_numbers(
        Graph& g, CoreMap c, EdgeWeightMap wm, VertexIndexMap vim,
        CoreNumVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::are_tagged_arguments<
                EdgeWeightMap, VertexIndexMap, CoreNumVisitor
            >,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        return detail::core_numbers_dispatch(g,c,wm,vim,vis,mpl::true_());
    }

    // non-named parameter version for the unweighted case
    template <typename Graph, typename CoreMap, typename CoreNumVisitor>
    inline typename property_traits<CoreMap>::value_type
    core_numbers(
        Graph& g, CoreMap c, CoreNumVisitor vis
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
        , typename boost::disable_if<
            parameter::is_argument_pack<CoreNumVisitor>,
            mpl::true_
        >::type = mpl::true_()
#endif
    )
    {
        return detail::core_numbers_dispatch(
            g,
            c,
            dummy_property_map(),
            get(vertex_index, g),
            vis,
            mpl::false_()
        );
    }

    // non-named paramter version for the unweighted case
    template <typename Graph, typename CoreMap>
    inline typename property_traits<CoreMap>::value_type
    core_numbers(Graph& g, CoreMap c)
    {
        return core_numbers(g, c, make_core_numbers_visitor(null_visitor()));
    }

    // non-named parameter version for the weighted case
//    template <typename Graph, typename CoreMap, typename EdgeWeightMap>
//    typename property_traits<CoreMap>::value_type
//    core_numbers(Graph& g, CoreMap c, EdgeWeightMap wm)
//    {
//        typedef typename graph_traits<Graph>::vertices_size_type size_type;
//        detail::compute_in_degree_map(g,c,wm);
//        return detail::core_numbers_dispatch(g,c,wm,get(vertex_index,g),
//            make_core_numbers_visitor(null_visitor()));
//    }

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    // named parameter version for all cases
    template <typename Graph, typename CoreMap, typename Args>
    inline typename property_traits<CoreMap>::value_type
    core_numbers(
        Graph& g, CoreMap c, Args const& args,
        typename boost::enable_if<
            parameter::is_argument_pack<Args>,
            mpl::true_
        >::type = mpl::true_()
    )
    {
        default_core_numbers_visitor default_visitor;
        typename boost::remove_const<
            typename parameter::value_type<
                Args,
                boost::graph::keywords::tag::visitor,
                default_core_numbers_visitor
            >::type
        >::type vis = args[boost::graph::keywords::_visitor | default_visitor];
        return detail::core_numbers_dispatch(
            g,
            c,
            detail::override_const_property(
                args,
                boost::graph::keywords::_weight_map,
                g,
                edge_weight
            ),
            detail::override_const_property(
                args,
                boost::graph::keywords::_vertex_index_map,
                g,
                vertex_index
            ),
            vis,
            typename mpl::has_key<
                Args,
                boost::graph::keywords::tag::weight_map
            >::type()
        );
    }

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
    template < \
        typename Graph, typename CoreMap, typename TA \
        BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA) \
    > \
    inline void name( \
        Graph& g, CoreMap c, const TA& ta \
        BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
        typename boost::enable_if< \
            parameter::are_tagged_arguments< \
                TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
            >, mpl::true_ \
        >::type = mpl::true_() \
    ) \
    { \
        name( \
            g, c, \
            parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta)) \
        ); \
    }

BOOST_PP_REPEAT_FROM_TO(1, 4, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, core_numbers)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS

    template <typename Graph, typename CoreMap, typename CoreNumVisitor>
    inline typename property_traits<CoreMap>::value_type
    weighted_core_numbers(Graph& g, CoreMap c, CoreNumVisitor vis)
    {
        return core_numbers(g,c,get(edge_weight,g),get(vertex_index,g),vis);
    }

    template <typename Graph, typename CoreMap>
    inline typename property_traits<CoreMap>::value_type
    weighted_core_numbers(Graph& g, CoreMap c)
    {
        return weighted_core_numbers(
            g,c, make_core_numbers_visitor(null_visitor())
        );
    }
} // namespace boost

#endif // BOOST_GRAPH_CORE_NUMBERS_HPP

