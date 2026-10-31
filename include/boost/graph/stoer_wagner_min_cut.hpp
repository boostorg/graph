//            Copyright Daniel Trebbien 2010.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_STOER_WAGNER_MIN_CUT_HPP
#define BOOST_GRAPH_STOER_WAGNER_MIN_CUT_HPP 1

#include <boost/assert.hpp>
#include <set>
#include <vector>
#include <boost/concept_check.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/buffer_concepts.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/utility/result_of.hpp>
#include <boost/graph/iteration_macros.hpp>

#include <functional>

namespace boost
{

namespace detail
{
    // Records the phase cut: the last two vertices swept and the reach count of the last.
    template < class UndirectedGraph, class KeyMap, class WeightType >
    struct mas_phase_recorder : public boost::graph::default_mas_visitor
    {
        using vertex_descriptor = typename boost::graph_traits< UndirectedGraph >::vertex_descriptor;

        mas_phase_recorder(vertex_descriptor& s, vertex_descriptor& t, WeightType& w, KeyMap keys)
        : s_(s), t_(t), w_(w), keys_(keys) {}

        void start_vertex(vertex_descriptor u, const UndirectedGraph&)
        {
            s_ = t_;
            t_ = u;
            w_ = get(keys_, u);
        }

        vertex_descriptor& s_;
        vertex_descriptor& t_;
        WeightType& w_;
        KeyMap keys_;
    };

    /**
     * \brief Performs a phase of the Stoer-Wagner min-cut algorithm
     *
     * Performs a phase of the Stoer-Wagner min-cut algorithm.
     *
     * As described by Stoer & Wagner (1997), a phase is simply a maximum
     * adjacency search (also called a maximum cardinality search), which
     * results in the selection of two vertices \em s and \em t, and, as a side
     * product, a minimum <em>s</em>-<em>t</em> cut of the input graph. Here,
     * the input graph is basically \p g, but some vertices are virtually
     * assigned to others as a way of viewing \p g as a graph with some sets of
     * vertices merged together.
     *
     * This implementation is a translation of pseudocode by Professor Uri
     * Zwick, School of Computer Science, Tel Aviv University.
     *
     * \pre \p g is a connected, undirected graph
     * \param[in] g the input graph
     * \param[in] assignments a read/write property map from each vertex to the
     *                        vertex that it is assigned to
     * \param[in] assignedVertices a list of vertices that are assigned to
     *                             others
     * \param[in] weights a readable property map from each edge to its
     *                    weight (a non-negative value)
     * \param[out] pq a keyed, updatable max-priority queue
     * \returns a tuple (\em s, \em t, \em w) of the "<em>s</em>" and
     *          "<em>t</em>" of the minimum <em>s</em>-<em>t</em> cut and the
     *          cut weight \em w of the minimum <em>s</em>-<em>t</em> cut.
     * \see http://www.cs.tau.ac.il/~zwick/grad-algo-08/gmc.pdf
     *
     * \author Daniel Trebbien
     * \date 2010-09-11
     */
    template < class UndirectedGraph, class VertexAssignmentMap,
        class WeightMap, class KeyedUpdatablePriorityQueue >
    boost::tuple<
        typename boost::graph_traits< UndirectedGraph >::vertex_descriptor,
        typename boost::graph_traits< UndirectedGraph >::vertex_descriptor,
        typename boost::property_traits< WeightMap >::value_type >
    stoer_wagner_phase(const UndirectedGraph& g,
        VertexAssignmentMap assignments,
        const std::set< typename boost::graph_traits<
            UndirectedGraph >::vertex_descriptor >& assignedVertices,
        WeightMap weights, KeyedUpdatablePriorityQueue& pq)
    {
        typedef
            typename boost::graph_traits< UndirectedGraph >::vertex_descriptor
                vertex_descriptor;
        typedef typename boost::property_traits< WeightMap >::value_type
            weight_type;

        BOOST_ASSERT(pq.empty());
        typename KeyedUpdatablePriorityQueue::key_map keys = pq.keys();

        BGL_FORALL_VERTICES_T(v, g, UndirectedGraph)
        {
            if (v == get(assignments, v))
            { // foreach u \in V do
                put(keys, v, weight_type(0));

                pq.push(v);
            }
        }

        BOOST_ASSERT(pq.size() >= 2);

        // the cut of the phase is the last two vertices the maximum adjacency
        // sweep visits and the reach count of the last one
        vertex_descriptor s = boost::graph_traits< UndirectedGraph >::null_vertex();
        vertex_descriptor t = boost::graph_traits< UndirectedGraph >::null_vertex();
        weight_type w = weight_type(0);

        using vis_t = mas_phase_recorder< UndirectedGraph, typename KeyedUpdatablePriorityQueue::key_map, weight_type >;
        vis_t recorder(s, t, w, keys);

        boost::graph::mas_detail::mas_sweep(g, weights, recorder, assignments, assignedVertices, pq);

        return boost::make_tuple(s, t, w);
    }

    /**
     * \brief Computes a min-cut of the input graph
     *
     * Computes a min-cut of the input graph using the Stoer-Wagner algorithm.
     *
     * \pre \p g is a connected, undirected graph
     * \pre <code>pq.empty()</code>
     * \param[in] g the input graph
     * \param[in] weights a readable property map from each edge to its weight
     * (a non-negative value) \param[out] parities a writable property map from
     * each vertex to a bool type object for distinguishing the two vertex sets
     * of the min-cut \param[out] assignments a read/write property map from
     * each vertex to a \c vertex_descriptor object. This map serves as work
     * space, and no particular meaning should be derived from property values
     *     after completion of the algorithm.
     * \param[out] pq a keyed, updatable max-priority queue
     * \returns the cut weight of the min-cut
     * \see
     * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.114.6687&rep=rep1&type=pdf
     * \see
     * http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.31.614&rep=rep1&type=pdf
     *
     * \author Daniel Trebbien
     * \date 2010-09-11
     */
    template < class UndirectedGraph, class WeightMap, class ParityMap,
        class VertexAssignmentMap, class KeyedUpdatablePriorityQueue,
        class IndexMap >
    typename boost::property_traits< WeightMap >::value_type
    stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights,
        ParityMap parities, VertexAssignmentMap assignments,
        KeyedUpdatablePriorityQueue& pq, IndexMap index_map)
    {
        typedef
            typename boost::graph_traits< UndirectedGraph >::vertex_descriptor
                vertex_descriptor;
        typedef typename boost::property_traits< WeightMap >::value_type
            weight_type;
        typedef
            typename boost::graph_traits< UndirectedGraph >::vertices_size_type
                vertices_size_type;
        typedef typename boost::property_traits< ParityMap >::value_type
            parity_type;

        vertices_size_type n = num_vertices(g);

        std::set< vertex_descriptor > assignedVertices;

        // initialize `assignments` (all vertices are initially assigned to
        // themselves)
        BGL_FORALL_VERTICES_T(v, g, UndirectedGraph) { put(assignments, v, v); }

        vertex_descriptor s, t;
        weight_type bestW;

        boost::tie(s, t, bestW) = boost::detail::stoer_wagner_phase(
            g, assignments, assignedVertices, weights, pq);
        BOOST_ASSERT(s != t);
        BGL_FORALL_VERTICES_T(v, g, UndirectedGraph)
        {
            put(parities, v, parity_type(v == t ? 1 : 0));
        }
        put(assignments, t, s);
        assignedVertices.insert(t);
        --n;

        for (; n >= 2; --n)
        {
            weight_type w;
            boost::tie(s, t, w) = boost::detail::stoer_wagner_phase(
                g, assignments, assignedVertices, weights, pq);
            BOOST_ASSERT(s != t);

            if (w < bestW)
            {
                BGL_FORALL_VERTICES_T(v, g, UndirectedGraph)
                {
                    put(parities, v,
                        parity_type(get(assignments, v) == t ? 1 : 0));

                    if (get(assignments, v)
                        == t) // all vertices that were assigned to t are now
                              // assigned to s
                        put(assignments, v, s);
                }

                bestW = w;
            }
            else
            {
                BGL_FORALL_VERTICES_T(v, g, UndirectedGraph)
                {
                    if (get(assignments, v)
                        == t) // all vertices that were assigned to t are now
                              // assigned to s
                        put(assignments, v, s);
                }
            }
            put(assignments, t, s);
            assignedVertices.insert(t);
        }

        BOOST_ASSERT(pq.empty());

        return bestW;
    }
} // end `namespace detail` within `namespace boost`

namespace graph
{
    // Fully positional 6-args
    template < class UndirectedGraph, class WeightMap, class ParityMap,
        class VertexAssignmentMap, class KeyedUpdatablePriorityQueue,
        class IndexMap >
    typename boost::property_traits< WeightMap >::value_type
    stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights,
        ParityMap parities, VertexAssignmentMap assignments,
        KeyedUpdatablePriorityQueue& pq, IndexMap index_map)
    {
        using vertex_descriptor = typename boost::graph_traits< UndirectedGraph >::vertex_descriptor;
        using vertices_size_type = typename boost::graph_traits< UndirectedGraph >::vertices_size_type;
        using edge_descriptor = typename boost::graph_traits< UndirectedGraph >::edge_descriptor;

        BOOST_CONCEPT_ASSERT((boost::IncidenceGraphConcept< UndirectedGraph >));
        BOOST_CONCEPT_ASSERT((boost::VertexListGraphConcept< UndirectedGraph >));
        BOOST_CONCEPT_ASSERT((boost::Convertible< typename boost::graph_traits< UndirectedGraph >::directed_category, boost::undirected_tag >));
        BOOST_CONCEPT_ASSERT((boost::ReadablePropertyMapConcept< WeightMap, edge_descriptor >));
        BOOST_CONCEPT_ASSERT((boost::WritablePropertyMapConcept< ParityMap, vertex_descriptor >));
        BOOST_CONCEPT_ASSERT((boost::ReadWritePropertyMapConcept< VertexAssignmentMap, vertex_descriptor >));
        BOOST_CONCEPT_ASSERT((boost::Convertible< vertex_descriptor, typename boost::property_traits< VertexAssignmentMap >::value_type >));
        BOOST_CONCEPT_ASSERT((boost::KeyedUpdatableQueueConcept< KeyedUpdatablePriorityQueue >));

        const vertices_size_type n = num_vertices(g);
        if (n < 2)
            throw boost::bad_graph("the input graph must have at least two vertices.");
        else if (!pq.empty())
            throw std::invalid_argument("the max-priority queue must be empty initially.");

        return boost::detail::stoer_wagner_min_cut(g, weights, parities, assignments, pq, index_map);
    }

    // Positional 5-args
    template < class UndirectedGraph, class WeightMap, class ParityMap,
        class VertexAssignmentMap, class KeyedUpdatablePriorityQueue >
    typename boost::property_traits< WeightMap >::value_type
    stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights,
        ParityMap parities, VertexAssignmentMap assignments,
        KeyedUpdatablePriorityQueue& pq)
    {
        return boost::graph::stoer_wagner_min_cut(g, weights, parities, assignments, pq, get(vertex_index, g));
    }

    // Positional 3-args. 
    template < class UndirectedGraph, class WeightMap, class ParityMap >
    typename boost::property_traits< WeightMap >::value_type
    stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights, ParityMap parities)
    {
        using vertex_descriptor = typename boost::graph_traits< UndirectedGraph >::vertex_descriptor;
        using weight_type = typename boost::property_traits< WeightMap >::value_type;
        using vertex_index_map_type = typename boost::property_map< UndirectedGraph, boost::vertex_index_t >::const_type;
        using distance_map_type = boost::shared_array_property_map< weight_type, vertex_index_map_type >;
        using index_in_heap_type = typename std::vector< vertex_descriptor >::size_type;
        using index_in_heap_map_type = boost::shared_array_property_map< index_in_heap_type, vertex_index_map_type >;
        using priority_queue_type = boost::d_ary_heap_indirect< vertex_descriptor, 4, index_in_heap_map_type, distance_map_type, std::greater< weight_type > >;
        using assignment_map_type = boost::shared_array_property_map< vertex_descriptor, vertex_index_map_type >;

        const vertex_index_map_type vertex_index_map = get(boost::vertex_index, g);
        distance_map_type distance_map = boost::make_shared_array_property_map(num_vertices(g), weight_type(0), vertex_index_map);
        index_in_heap_map_type index_in_heap_map = boost::make_shared_array_property_map(num_vertices(g), index_in_heap_type(-1), vertex_index_map);
        priority_queue_type pq(distance_map, index_in_heap_map);
        assignment_map_type assignment_map = boost::make_shared_array_property_map(num_vertices(g), vertex_descriptor(), vertex_index_map);

        return boost::graph::stoer_wagner_min_cut(g, weights, parities, assignment_map, pq, vertex_index_map);
    }

    namespace detail
    {
        template < class UndirectedGraph, class WeightMap >
        struct stoer_wagner_min_cut_impl
        {
            using result_type = typename boost::property_traits< WeightMap >::value_type;
            template < typename ArgPack >
            result_type operator()(const UndirectedGraph& g, WeightMap weights, const ArgPack& arg_pack) const
            {
                using namespace boost::graph::keywords;
                using vertex_descriptor = typename boost::graph_traits< UndirectedGraph >::vertex_descriptor;
                using weight_type = typename boost::property_traits< WeightMap >::value_type;
                using gen_type = boost::detail::make_priority_queue_from_arg_pack_gen< boost::graph::keywords::tag::max_priority_queue, weight_type, vertex_descriptor, std::greater< weight_type > >;

                gen_type gen(choose_param(get_param(arg_pack, boost::distance_zero_t()), weight_type(0)));
                typename boost::result_of< gen_type(const UndirectedGraph&, const ArgPack&) >::type pq = gen(g, arg_pack);

                boost::dummy_property_map dummy_prop;
                return boost::graph::stoer_wagner_min_cut(g, weights,
                    arg_pack[_parity_map | dummy_prop],
                    boost::detail::make_property_map_from_arg_pack_gen< tag::vertex_assignment_map, vertex_descriptor >(vertex_descriptor())(g, arg_pack),
                    pq,
                    boost::detail::override_const_property(arg_pack, _vertex_index_map, g, vertex_index));
            }
        };
    }
    // Generates the tagged-keyword Boost.Parameter overloads dispatching to stoer_wagner_min_cut_impl.
    BOOST_GRAPH_MAKE_FORWARDING_FUNCTION(stoer_wagner_min_cut, 2, 4)
} // end `namespace graph`

// -- deprecated overloads --

template < class UndirectedGraph, class WeightMap, class ParityMap,
    class VertexAssignmentMap, class KeyedUpdatablePriorityQueue, class IndexMap >
BOOST_DEPRECATED("use the positional boost::graph::stoer_wagner_min_cut. Removal planned for Boost 1.95.")
typename boost::property_traits< WeightMap >::value_type
stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights, ParityMap parities,
    VertexAssignmentMap assignments, KeyedUpdatablePriorityQueue& pq, IndexMap index_map)
{
    return graph::stoer_wagner_min_cut(g, weights, parities, assignments, pq, index_map);
}

// Named parameter interface
template < class UndirectedGraph, class WeightMap, class P, class T, class R >
BOOST_DEPRECATED("the named parameter interface is deprecated, use the positional boost::graph::stoer_wagner_min_cut. Removal planned for Boost 1.95.")
typename boost::property_traits< WeightMap >::value_type
stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights, const bgl_named_params< P, T, R >& params)
{
    using params_type = bgl_named_params< P, T, R >;
    // Converts the legacy bgl_named_params into the modern Boost.Parameter arg_pack the forwarding function expects.
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)
    return graph::stoer_wagner_min_cut_with_named_params(g, weights, arg_pack);
}

template < class UndirectedGraph, class WeightMap >
BOOST_DEPRECATED("the named parameter interface is deprecated, use the positional boost::graph::stoer_wagner_min_cut. Removal planned for Boost 1.95.")
typename boost::property_traits< WeightMap >::value_type
stoer_wagner_min_cut(const UndirectedGraph& g, WeightMap weights)
{
    // Converts the legacy bgl_named_params into the modern Boost.Parameter arg_pack the forwarding function expects.
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(boost::no_named_parameters, boost::no_named_parameters())
    return graph::stoer_wagner_min_cut_with_named_params(g, weights, arg_pack);
}
} // end `namespace boost`

#include <boost/graph/iteration_macros_undef.hpp>

#endif // !BOOST_GRAPH_STOER_WAGNER_MIN_CUT_HPP
