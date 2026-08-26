//
//=======================================================================
// Copyright 2012 Fernando Vilas
//           2010 Daniel Trebbien
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

// The maximum adjacency search algorithm was originally part of the
// Stoer-Wagner min cut implementation by Daniel Trebbien. It has been
// broken out into its own file to be a public search algorithm, with
// visitor concepts.
#ifndef BOOST_GRAPH_MAXIMUM_ADJACENCY_SEARCH_H
#define BOOST_GRAPH_MAXIMUM_ADJACENCY_SEARCH_H

/**
 * This is an implementation of the maximum adjacency search on an
 * undirected graph. It allows a visitor object to perform some
 * operation on each vertex as that vertex is visited.
 *
 * The algorithm runs as follows:
 *
 * Initialize all nodes to be unvisited (reach count = 0)
 *   and call vis.initialize_vertex
 * For i = number of nodes in graph downto 1
 *   Select the unvisited node with the highest reach count
 *     The user provides the starting node to break the first tie,
 *     but future ties are broken arbitrarily
 *   Visit the node by calling vis.start_vertex
 *   Increment the reach count for all unvisited neighbors
 *     and call vis.examine_edge for each of these edges
 *   Mark the node as visited and call vis.finish_vertex
 *
 */

#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/buffer_concepts.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/tuple/tuple.hpp>

#include <functional>
#include <set>
#include <vector>

namespace boost
{

template < class Visitor, class Graph > 
struct MASVisitorConcept
{
    void constraints()
    {
        boost::function_requires< boost::CopyConstructibleConcept< Visitor > >();
        vis.initialize_vertex(u, g);
        vis.start_vertex(u, g);
        vis.examine_edge(e, g);
        vis.finish_vertex(u, g);
    }
    Visitor vis;
    Graph g;
    typename boost::graph_traits< Graph >::vertex_descriptor u;
    typename boost::graph_traits< Graph >::edge_descriptor e;
};

template < class Visitors = null_visitor > 
class mas_visitor
{
public:
    mas_visitor() {}
    mas_visitor(Visitors vis) : m_vis(vis) {}

    template < class Vertex, class Graph >
    void initialize_vertex(Vertex u, Graph& g)
    {
        invoke_visitors(m_vis, u, g, ::boost::on_initialize_vertex());
    }

    template < class Vertex, class Graph > 
    void start_vertex(Vertex u, Graph& g)
    {
        invoke_visitors(m_vis, u, g, ::boost::on_start_vertex());
    }

    template < class Edge, class Graph > 
    void examine_edge(Edge e, Graph& g)
    {
        invoke_visitors(m_vis, e, g, ::boost::on_examine_edge());
    }

    template < class Vertex, class Graph >
    void finish_vertex(Vertex u, Graph& g)
    {
        invoke_visitors(m_vis, u, g, ::boost::on_finish_vertex());
    }

    BOOST_GRAPH_EVENT_STUB(on_initialize_vertex, mas)
    BOOST_GRAPH_EVENT_STUB(on_start_vertex, mas)
    BOOST_GRAPH_EVENT_STUB(on_examine_edge, mas)
    BOOST_GRAPH_EVENT_STUB(on_finish_vertex, mas)

protected:
    Visitors m_vis;
};

template < class Visitors >
mas_visitor< Visitors > make_mas_visitor(Visitors vis)
{
    return mas_visitor< Visitors >(vis);
}

typedef mas_visitor<> default_mas_visitor;

namespace graph
{
namespace detail
{

// Maximum adjacency sweep over an already populated queue. 
// Shared engine behind both maximum_adjacency_search and stoer_wagner_min_cut.
// The graph may be contracted through assignments (each vertex maps to its representative)
// with assigned_vertices listing the contracted vertices. 
// Pass an identity map and an empty set for no contraction. 
template <class Graph, class WeightMap, class MASVisitor, class VertexAssignmentMap, class KeyedUpdatablePriorityQueue>
void mas_sweep(
    const Graph& g, 
    WeightMap weight_map,
    MASVisitor vis,
    VertexAssignmentMap assignment_map,
    const std::set<typename boost::graph_traits< Graph >::vertex_descriptor >& assigned_vertices,
    KeyedUpdatablePriorityQueue& pq)
{
    // for an unvisited vertex, reach count is the total weight of its edges 
    // to the set of already-visited vertices.
    // That is, how strongly that vertex is pulled toward what's been visited so far.

    // reach counts are the queue keys.
    auto key_map = pq.keys();

    while (!pq.empty())
    {
        // extract max: top then pop
        const auto u = pq.top();
        vis.start_vertex(u, g);
        pq.pop();

        for (const auto& e : make_iterator_range(out_edges(u, g)))
        {
            vis.examine_edge(e, g);
            // map the target to itself or its super node
            const auto v = get(assignment_map, target(e, g));
            // in the queue means still unvisited
            if (pq.contains(v))
            {
                // increase key: reach count plus the edge weight
                put(key_map, v, get(key_map, v) + get(weight_map, e));
                pq.update(v);
            }
        }

        // also relax the edges of every vertex contracted into u
        for (const auto& member : assigned_vertices)
        {
            // keep only vertices belonging to this supernode
            if (get(assignment_map, member) != u)
                continue;

            for (const auto& e : make_iterator_range(out_edges(member, g)))
            {
                vis.examine_edge(e, g);
                // map the target to itself or its super node
                const auto v = get(assignment_map, target(e, g));
                if (pq.contains(v))
                {
                    put(key_map, v, get(key_map, v) + get(weight_map, e));
                    pq.update(v);
                }
            }
        }
        vis.finish_vertex(u, g);
    }
}
} // namespace detail

// Public maximum adjacency search. 
// Seeds the queue, gives the start vertex the
// highest priority, then runs the shared sweep with no contraction.
template < class Graph, class WeightMap, class MASVisitor, class KeyedUpdatablePriorityQueue >
void maximum_adjacency_search(
    const Graph& g,
    WeightMap weight_map,
    MASVisitor vis,
    const typename boost::graph_traits< Graph >::vertex_descriptor start,
    KeyedUpdatablePriorityQueue pq)
{
    using vertex_descriptor = typename boost::graph_traits< Graph >::vertex_descriptor;
    using weight_type = typename boost::property_traits< WeightMap >::value_type;
    using directed_category = typename boost::graph_traits< Graph >::directed_category;
    using edge_descriptor = typename boost::graph_traits< Graph >::edge_descriptor;
    
    BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept< Graph >));
    BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept< Graph >));
    BOOST_CONCEPT_ASSERT((boost::Convertible< directed_category, boost::undirected_tag >));
    BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept< WeightMap, edge_descriptor >));
    boost::function_requires< MASVisitorConcept< MASVisitor, Graph > >();
    BOOST_CONCEPT_ASSERT((boost::KeyedUpdatableQueueConcept< KeyedUpdatablePriorityQueue >));

    if (num_vertices(g) < 2)
        throw boost::bad_graph("the input graph must have at least two vertices.");
    if (!pq.empty())
        throw std::invalid_argument("the priority queue must be empty initially.");

    // reach counts are the queue keys
    auto key_map = pq.keys();

    // seed every vertex with reach count 0
    for (const auto& v : make_iterator_range(vertices(g)))
    {
        put(key_map, v, static_cast< weight_type >(0));
        vis.initialize_vertex(v, g);
        pq.push(v);
    }
    BOOST_ASSERT(pq.size() >= 2);

    // give the start vertex the highest priority
    put(key_map, start, get(key_map, start) + num_vertices(g) + 1);
    pq.update(start);

    // no contraction: identity assignment map and empty contracted set
    const boost::typed_identity_property_map< vertex_descriptor > identity_map;
    const std::set< vertex_descriptor > no_assigned_vertices;
    detail::mas_sweep(g, weight_map, vis, identity_map, no_assigned_vertices, pq);
}

// Convenience overload that defaults only the priority queue. Building the
// queue is the cumbersome part, so this spares the caller that while still
// letting them choose the start vertex.
template < class Graph, class WeightMap, class MASVisitor >
void maximum_adjacency_search(const Graph& g, WeightMap weights, MASVisitor vis,
    const typename boost::graph_traits< Graph >::vertex_descriptor start)
{
    using vertex_descriptor = typename boost::graph_traits< Graph >::vertex_descriptor;
    using weight_type = typename boost::property_traits< WeightMap >::value_type;
    using vertex_index_map_type = typename boost::property_map< Graph, boost::vertex_index_t >::const_type;
    using distance_map_type = boost::shared_array_property_map< weight_type, vertex_index_map_type >;
    using index_in_heap_type = typename std::vector< vertex_descriptor >::size_type;
    using index_in_heap_map_type = boost::shared_array_property_map< index_in_heap_type, vertex_index_map_type >;
    using priority_queue_type = boost::d_ary_heap_indirect< vertex_descriptor, 4, index_in_heap_map_type, distance_map_type, std::greater< weight_type > >;

    const vertex_index_map_type vertex_index_map = get(boost::vertex_index, g);
    distance_map_type distance_map = boost::make_shared_array_property_map(num_vertices(g), weight_type(0), vertex_index_map);
    index_in_heap_map_type index_in_heap_map = boost::make_shared_array_property_map(num_vertices(g), index_in_heap_type(-1), vertex_index_map);
    priority_queue_type pq(distance_map, index_in_heap_map);

    maximum_adjacency_search(g, weights, vis, start, pq);
}

// Convenience overload that additionally defaults the start vertex to the first
// vertex, matching what the deprecated named parameter interface built.
template < class Graph, class WeightMap, class MASVisitor >
void maximum_adjacency_search(const Graph& g, WeightMap weights, MASVisitor vis)
{
    maximum_adjacency_search(g, weights, vis, *vertices(g).first);
}

} // namespace graph

// -- old interface to be deprecated --

template < class Graph, class WeightMap, class MASVisitor, class VertexAssignmentMap, class KeyedUpdatablePriorityQueue >
BOOST_DEPRECATED("assignments is unused, use the 5-argument boost::graph::maximum_adjacency_search. Removal planned for Boost 1.95.")
void maximum_adjacency_search(
    const Graph& g,
    WeightMap weights,
    MASVisitor vis,
    const typename boost::graph_traits< Graph >::vertex_descriptor start,
    VertexAssignmentMap,
    KeyedUpdatablePriorityQueue pq)
{
    using vertex_descriptor = typename boost::graph_traits< Graph >::vertex_descriptor;
    BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept< VertexAssignmentMap, vertex_descriptor >));
    graph::maximum_adjacency_search(g, weights, vis, start, pq);
}

namespace graph
{
    namespace detail
    {
        template < typename WeightMap > struct mas_dispatch
        {
            typedef void result_type;
            template < typename Graph, typename ArgPack >
            static result_type apply(const Graph& g,
                // const bgl_named_params<P,T,R>& params,
                const ArgPack& params, WeightMap w)
            {

                using namespace boost::graph::keywords;
                typedef typename boost::graph_traits< Graph >::vertex_descriptor
                    vertex_descriptor;
                typedef typename WeightMap::value_type weight_type;

                typedef boost::detail::make_priority_queue_from_arg_pack_gen<
                    boost::graph::keywords::tag::max_priority_queue,
                    weight_type, vertex_descriptor,
                    std::greater< weight_type > >
                    default_pq_gen_type;

                default_pq_gen_type pq_gen(
                    choose_param(get_param(params, boost::distance_zero_t()),
                        weight_type(0)));

                typename boost::result_of< default_pq_gen_type(
                    const Graph&, const ArgPack&) >::type pq
                    = pq_gen(g, params);

                boost::null_visitor null_vis;
                boost::mas_visitor< boost::null_visitor > default_visitor(
                    null_vis);
                vertex_descriptor v = vertex_descriptor();
                boost::detail::make_property_map_from_arg_pack_gen<
                    boost::graph::keywords::tag::vertex_assignment_map,
                    vertex_descriptor >
                    map_gen(v);
                typename boost::detail::map_maker< Graph, ArgPack,
                    boost::graph::keywords::tag::vertex_assignment_map,
                    vertex_descriptor >::map_type default_map
                    = map_gen(g, params);
                boost::maximum_adjacency_search(g, w,
                    params[_visitor | default_visitor],
                    params[_root_vertex | *vertices(g).first],
                    params[_vertex_assignment_map | default_map], pq);
            }
        };

        template <> struct mas_dispatch< boost::param_not_found >
        {
            typedef void result_type;

            template < typename Graph, typename ArgPack >
            static result_type apply(
                const Graph& g, const ArgPack& params, param_not_found)
            {

                using namespace boost::graph::keywords;
                typedef typename boost::graph_traits< Graph >::vertex_descriptor
                    vertex_descriptor;

                // get edge_weight_t as the weight type
                typedef typename boost::property_map< Graph, edge_weight_t >
                    WeightMap;
                typedef typename WeightMap::value_type weight_type;

                typedef boost::detail::make_priority_queue_from_arg_pack_gen<
                    boost::graph::keywords::tag::max_priority_queue,
                    weight_type, vertex_descriptor,
                    std::greater< weight_type > >
                    default_pq_gen_type;

                default_pq_gen_type pq_gen(
                    choose_param(get_param(params, boost::distance_zero_t()),
                        weight_type(0)));

                typename boost::result_of< default_pq_gen_type(
                    const Graph&, const ArgPack&) >::type pq
                    = pq_gen(g, params);

                boost::null_visitor null_vis;
                boost::mas_visitor< boost::null_visitor > default_visitor(
                    null_vis);
                vertex_descriptor v = vertex_descriptor();
                boost::detail::make_property_map_from_arg_pack_gen<
                    boost::graph::keywords::tag::vertex_assignment_map,
                    vertex_descriptor >
                    map_gen(v);
                typename boost::detail::map_maker< Graph, ArgPack,
                    boost::graph::keywords::tag::vertex_assignment_map,
                    vertex_descriptor >::map_type default_map
                    = map_gen(g, params);
                boost::maximum_adjacency_search(g, get(edge_weight, g),
                    params[_visitor | default_visitor],
                    params[_root_vertex | *vertices(g).first],
                    params[_vertex_assignment_map | default_map], pq);
            }
        };
    } // end namespace detail
} // end namespace graph

// Named parameter interface
// BOOST_GRAPH_MAKE_OLD_STYLE_PARAMETER_FUNCTION(maximum_adjacency_search, 1)
template < typename Graph, typename P, typename T, typename R >
BOOST_DEPRECATED("the named parameter interface is deprecated, use the positional boost::graph::maximum_adjacency_search. Removal planned for Boost 1.95.")
void maximum_adjacency_search(
    const Graph& g, const bgl_named_params< P, T, R >& params)
{

    typedef bgl_named_params< P, T, R > params_type;
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)

    // do the dispatch based on WeightMap
    typedef typename get_param_type< edge_weight_t,
        bgl_named_params< P, T, R > >::type W;
    graph::detail::mas_dispatch< W >::apply(
        g, arg_pack, get_param(params, edge_weight));
}

namespace graph
{
    namespace detail
    {
        template < typename Graph > struct maximum_adjacency_search_impl
        {
            typedef void result_type;

            template < typename ArgPack >
            void operator()(const Graph& g, const ArgPack& arg_pack) const
            {
                // call the function that does the dispatching
                typedef
                    typename get_param_type< edge_weight_t, ArgPack >::type W;
                graph::detail::mas_dispatch< W >::apply(
                    g, arg_pack, get_param(arg_pack, edge_weight));
            }
        };
    } // end namespace detail

    BOOST_GRAPH_MAKE_FORWARDING_FUNCTION(maximum_adjacency_search, 1, 5)

} // end namespace graph
} // end namespace boost

#endif // BOOST_GRAPH_MAXIMUM_ADJACENCY_SEARCH_H
