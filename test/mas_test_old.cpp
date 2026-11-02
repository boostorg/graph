//            Copyright Fernando Vilas 2012.
//     Based on stoer_wagner_test.cpp by Daniel Trebbien.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)

// This test exercises the deprecated interfaces on purpose (the named
// parameter and assignment-map overloads, and the boost:: visitor aliases
// mas_visitor / make_mas_visitor / default_mas_visitor that now live in
// boost::graph), so silence the deprecation warnings.
#define BOOST_ALLOW_DEPRECATED_SYMBOLS

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>
#include <stdexcept>
#include <vector>
#include <string>
#include <boost/array.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/make_shared.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/tuple/tuple_comparison.hpp>
#include <boost/tuple/tuple_io.hpp>

#include <boost/graph/iteration_macros.hpp>

#include "mas_sw_maxflow_oracle.hpp"

using mas_sw_oracle::undirected_graph;
using mas_sw_oracle::weight_map_type;
using mas_sw_oracle::weight_type;
using mas_sw_oracle::vertex_descriptor;
using mas_sw_oracle::edge_descriptor;

typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >
    undirected_unweighted_graph;

std::string test_dir;

struct edge_t
{
    unsigned long first;
    unsigned long second;
};

template < typename Graph, typename KeyedUpdatablePriorityQueue >
class mas_test_visitor : public boost::default_mas_visitor
{
public:
    typedef typename boost::graph_traits< Graph >::vertex_descriptor
        vertex_descriptor;
    typedef typename KeyedUpdatablePriorityQueue::key_type weight_type;

    explicit mas_test_visitor(KeyedUpdatablePriorityQueue& pq)
    :   m_pq_(pq),
        vertex_visit_order_(boost::make_shared< std::vector< vertex_descriptor > >()),
        vertex_weights_when_visited_(boost::make_shared< std::vector< weight_type > >())
    {}

    void clear()
    {
        vertex_visit_order_->clear();
        vertex_weights_when_visited_->clear();
    }

    void start_vertex(vertex_descriptor u, const Graph&)
    {
        vertex_visit_order_->push_back(u);

        const weight_type u_weight = get(m_pq_.keys(), u);
        vertex_weights_when_visited_->push_back(u_weight);
    }

    const std::vector<vertex_descriptor>& vertex_visit_order() const {
        return *vertex_visit_order_;
    }

    const std::vector<weight_type>& vertex_weights_when_visited() const {
        return *vertex_weights_when_visited_;
    }

private:
    const KeyedUpdatablePriorityQueue& m_pq_;
    boost::shared_ptr< std::vector< vertex_descriptor > > vertex_visit_order_;
    boost::shared_ptr< std::vector< weight_type > > vertex_weights_when_visited_;
};

// the example from Stoer & Wagner (1997)
// Check various implementations of the ArgPack where
// the weights are provided in it, and one case where
// they are not.
void test0()
{
    typedef boost::graph_traits< undirected_graph >::vertex_descriptor
        vertex_descriptor;
    typedef boost::graph_traits< undirected_graph >::edge_descriptor
        edge_descriptor;

    boost::array< edge_t, 12 > edge_list = { { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 4 }, { 1, 4 }, { 1, 5 },
            { 2, 6 }, { 3, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 } } };
    const boost::array<weight_type, 12> ws = { 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 1, 3 };
    const std::size_t vertices_count = 8;

    undirected_graph g(edge_list.cbegin(), edge_list.cend(), ws.cbegin(), vertices_count, ws.size());

    weight_map_type weights = get(boost::edge_weight, g);

    std::map< vertex_descriptor, vertex_descriptor > assignment;
    boost::associative_property_map<
        std::map< vertex_descriptor, vertex_descriptor > >
        assignments(assignment);

    typedef boost::shared_array_property_map< weight_type,
        boost::property_map< undirected_graph,
            boost::vertex_index_t >::const_type >
        distances_type;
    distances_type distances = boost::make_shared_array_property_map(
        num_vertices(g), weight_type(0), get(boost::vertex_index, g));
    typedef std::vector< vertex_descriptor >::size_type index_in_heap_type;
    typedef boost::shared_array_property_map< index_in_heap_type,
        boost::property_map< undirected_graph,
            boost::vertex_index_t >::const_type >
        indicesInHeap_type;
    indicesInHeap_type indicesInHeap = boost::make_shared_array_property_map(
        num_vertices(g), index_in_heap_type(-1), get(boost::vertex_index, g));
    boost::d_ary_heap_indirect< vertex_descriptor, 22, indicesInHeap_type,
        distances_type, std::greater< weight_type > >
        pq(distances, indicesInHeap);

    mas_test_visitor< undirected_graph,
        boost::d_ary_heap_indirect< vertex_descriptor, 22, indicesInHeap_type,
            distances_type, std::greater< weight_type > > >
        test_vis(pq);

    boost::maximum_adjacency_search(g,
        boost::weight_map(weights)
            .visitor(test_vis)
            .root_vertex(*vertices(g).first)
            .vertex_assignment_map(assignments)
            .max_priority_queue(pq));

    const boost::array< vertex_descriptor, vertices_count > expected_vertex_order1 = { 0, 4, 1, 5, 2, 3, 6, 7 };
    const boost::array< weight_type, vertices_count > expected_weights_when_visited1 = { 9, 3, 4, 5, 3, 4, 5, 5 };

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order1.cbegin(),
        expected_vertex_order1.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited1.cbegin(),
        expected_weights_when_visited1.cend()
    );

    test_vis.clear();

    boost::maximum_adjacency_search(g,
        boost::weight_map(weights)
            .visitor(test_vis)
            .root_vertex(*vertices(g).first)
            .max_priority_queue(pq));

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order1.cbegin(),
        expected_vertex_order1.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited1.cbegin(),
        expected_weights_when_visited1.cend()
    );

    test_vis.clear();

    boost::maximum_adjacency_search(
        g, boost::weight_map(weights).visitor(test_vis).max_priority_queue(pq));

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order1.cbegin(),
        expected_vertex_order1.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited1.cbegin(),
        expected_weights_when_visited1.cend()
    );

    test_vis.clear();

    boost::maximum_adjacency_search(g,
        boost::weight_map(weights).visitor(
            boost::make_mas_visitor(boost::null_visitor())));

    boost::maximum_adjacency_search(g, boost::weight_map(weights));

    boost::maximum_adjacency_search(g, boost::root_vertex(*vertices(g).first));

    test_vis.clear();
    boost::maximum_adjacency_search(g,
        boost::weight_map(
            boost::make_constant_property< edge_descriptor >(weight_type(1)))
            .visitor(test_vis)
            .max_priority_queue(pq));

    const boost::array< vertex_descriptor, vertices_count > expected_vertex_order2 = { 0, 1, 4, 5, 2, 6, 3, 7 };
    const boost::array< weight_type, vertices_count > expected_weights_when_visited2 = { 9, 1, 2, 2, 1, 2, 2, 2 };

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order2.cbegin(),
        expected_vertex_order2.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited2.cbegin(),
        expected_weights_when_visited2.cend()
    );
}

// Check the unweighted case
// with and without providing a weight_map
void test1()
{
    typedef boost::graph_traits<
        undirected_unweighted_graph >::vertex_descriptor vertex_descriptor;
    typedef boost::graph_traits< undirected_unweighted_graph >::edge_descriptor
        edge_descriptor;

    boost::array< edge_t, 12 > edge_list = { { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 4 }, { 1, 4 }, { 1, 5 },
            { 2, 6 }, { 3, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 } } };
    const std::size_t vertices_count = 8;

    undirected_unweighted_graph g(edge_list.cbegin(), edge_list.cend(), vertices_count);

    std::map< vertex_descriptor, vertex_descriptor > assignment;
    boost::associative_property_map<
        std::map< vertex_descriptor, vertex_descriptor > >
        assignments(assignment);

    typedef unsigned weight_type;
    typedef boost::shared_array_property_map< weight_type,
        boost::property_map< undirected_graph,
            boost::vertex_index_t >::const_type >
        distances_type;
    distances_type distances = boost::make_shared_array_property_map(
        num_vertices(g), weight_type(0), get(boost::vertex_index, g));
    typedef std::vector< vertex_descriptor >::size_type index_in_heap_type;
    typedef boost::shared_array_property_map< index_in_heap_type,
        boost::property_map< undirected_graph,
            boost::vertex_index_t >::const_type >
        indicesInHeap_type;
    indicesInHeap_type indicesInHeap = boost::make_shared_array_property_map(
        num_vertices(g), index_in_heap_type(-1), get(boost::vertex_index, g));
    boost::d_ary_heap_indirect< vertex_descriptor, 22, indicesInHeap_type,
        distances_type, std::greater< weight_type > >
        pq(distances, indicesInHeap);

    mas_test_visitor< undirected_unweighted_graph,
        boost::d_ary_heap_indirect< vertex_descriptor, 22, indicesInHeap_type,
            distances_type, std::greater< weight_type > > >
        test_vis(pq);

    boost::maximum_adjacency_search(g,
        boost::weight_map(
            boost::make_constant_property< edge_descriptor >(weight_type(1)))
            .visitor(test_vis)
            .max_priority_queue(pq));

    const boost::array< vertex_descriptor, vertices_count > expected_vertex_order1 = { 0, 1, 4, 5, 2, 6, 3, 7 };
    const boost::array< weight_type, vertices_count > expected_weights_when_visited1 = { 9, 1, 2, 2, 1, 2, 2, 2 };

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order1.cbegin(),
        expected_vertex_order1.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited1.cbegin(),
        expected_weights_when_visited1.cend()
    );

    test_vis.clear();

    const boost::array<weight_type, 12> ws = { 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 1, 3 };
    std::map< edge_descriptor, weight_type > wm;

    weight_type i = 0;
    BGL_FORALL_EDGES(e, g, undirected_unweighted_graph)
    {
        wm[e] = ws[i];
        ++i;
    }
    boost::associative_property_map< std::map< edge_descriptor, weight_type > >
        ws_map(wm);

    boost::maximum_adjacency_search(
        g, boost::weight_map(ws_map).visitor(test_vis).max_priority_queue(pq));

    const boost::array< vertex_descriptor, vertices_count > expected_vertex_order2 = { 0, 4, 1, 5, 2, 3, 6, 7 };
    const boost::array< weight_type, vertices_count > expected_weights_when_visited2 = { 9, 3, 4, 5, 3, 4, 5, 5 };

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order2.cbegin(),
        expected_vertex_order2.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited2.cbegin(),
        expected_weights_when_visited2.cend()
    );
}

typedef boost::graph_traits< undirected_unweighted_graph >::vertex_descriptor mas_test_vertex_descriptor;
typedef boost::graph_traits< undirected_unweighted_graph >::edge_descriptor mas_test_edge_descriptor;

typedef std::size_t mas_test_weight_type; // weight corresponds to the priority value in the priority queue.
typedef boost::shared_array_property_map< mas_test_weight_type, boost::property_map< undirected_graph, boost::vertex_index_t >::const_type > mas_test_distances_type;
typedef std::vector< mas_test_vertex_descriptor >::size_type mas_test_index_in_heap_type;
typedef boost::shared_array_property_map< mas_test_index_in_heap_type, boost::property_map< undirected_graph, boost::vertex_index_t >::const_type > mas_test_indicesInHeap_type;
const std::size_t mas_test_arity = 4;
typedef boost::d_ary_heap_indirect< mas_test_vertex_descriptor, mas_test_arity, mas_test_indicesInHeap_type, mas_test_distances_type, std::greater< mas_test_weight_type > > mas_test_maxheap_type;
typedef mas_test_visitor< undirected_unweighted_graph, mas_test_maxheap_type> mas_text_visitor_type;

template <typename Graph>
mas_test_maxheap_type create_mas_test_maxheap(const Graph& g) {
    mas_test_distances_type distances = boost::make_shared_array_property_map(
        num_vertices(g), mas_test_weight_type(0), get(boost::vertex_index, g));

    mas_test_indicesInHeap_type indicesInHeap = boost::make_shared_array_property_map(
        num_vertices(g), mas_test_index_in_heap_type(-1), get(boost::vertex_index, g));

    return mas_test_maxheap_type(distances, indicesInHeap);
}

template <std::size_t edge_count, std::size_t vertices_count>
void test_weighted(
        const boost::array<edge_t, edge_count>& edge_list,
        const boost::array<mas_test_weight_type, edge_count> weights_list,
        const boost::array<mas_test_vertex_descriptor, vertices_count>& expected_vertex_order,
        const boost::array<mas_test_weight_type, vertices_count>& expected_weights_when_visited,
        const mas_test_vertex_descriptor start_vertex = 0)
{
    const undirected_unweighted_graph g(edge_list.cbegin(), edge_list.cend(), vertices_count);

    mas_test_maxheap_type pq = create_mas_test_maxheap(g);
    mas_text_visitor_type test_vis = mas_text_visitor_type(pq);

    std::map< mas_test_edge_descriptor, mas_test_weight_type > weights_map;

    std::size_t i = 0;
    BGL_FORALL_EDGES(e, g, undirected_unweighted_graph)
    {
        weights_map[e] = weights_list[i];
        ++i;
    }
    boost::associative_property_map< std::map< mas_test_edge_descriptor, mas_test_weight_type > >
        weights_boost_map(weights_map);

    boost::maximum_adjacency_search(
        g,
        boost::weight_map(
            weights_boost_map)
            .visitor(test_vis)
            .max_priority_queue(pq)
            .root_vertex(start_vertex)
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_visit_order().begin(),
        test_vis.vertex_visit_order().end(),
        expected_vertex_order.cbegin(),
        expected_vertex_order.cend()
    );

    BOOST_TEST_ALL_EQ(
        test_vis.vertex_weights_when_visited().begin(),
        test_vis.vertex_weights_when_visited().end(),
        expected_weights_when_visited.cbegin(),
        expected_weights_when_visited.cend()
    );
}

template <std::size_t edge_count, std::size_t vertices_count>
void test_unweighted(
        const boost::array<edge_t, edge_count>& edge_list,
        const boost::array<mas_test_vertex_descriptor, vertices_count>& expected_vertex_order,
        const boost::array<mas_test_weight_type, vertices_count>& expected_weights_when_visited,
        const mas_test_vertex_descriptor start_vertex = 0)
{
    boost::array<mas_test_weight_type, edge_count> weights_list;
    for (std::size_t i = 0; i < edge_count; i++) {
        weights_list[i] = 1;
    }

    test_weighted(
        edge_list,
        weights_list,
        expected_vertex_order,
        expected_weights_when_visited,
        start_vertex);
}

void test2_noweights() {
    const std::size_t edge_count = 1;
    const std::size_t vertices_count = 2;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 } } };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 1 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 1 };

    test_unweighted(
        edge_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

void test3_noweights() {
    const std::size_t edge_count = 2;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 1, 2 } } };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 1, 2 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 1, 1 };

    test_unweighted(
        edge_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

void test4_noweights() {
    const std::size_t edge_count = 3;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 0, 2 }, { 1, 2 } } };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 1, 2 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 1, 2 };

    test_unweighted(
        edge_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

// The example graph from Matula (1993)
void test5_Matula1993() {
    const std::size_t edge_count = 24;
    const std::size_t vertices_count = 12;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 0, 2 },
        { 0, 3 }, { 0, 9 }, { 1, 2 }, { 1, 4 }, { 1, 10 }, { 2, 5 }, { 2, 11 },
        { 3, 4 }, { 3, 5 }, { 3, 6 }, { 4, 5 }, { 4, 7 }, { 5, 8 }, { 6, 7 },
        { 6, 8 }, { 6, 9 }, { 7, 8 }, { 7, 10 }, { 8, 11 }, { 9, 10 },
        { 9, 11 }, { 10, 11 } } };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 1, 2, 10, 9, 11, 6, 3, 7, 4, 8, 5 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 1, 2, 1, 2, 3, 1, 2, 2, 3, 3, 4 };

    test_unweighted(
        edge_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

// Testing with a different start vertex
void test6_noweights_start_vertex() {
    const std::size_t edge_count = 2;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 1, 2 } } };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 1, 0, 2 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 1, 1 };

    test_unweighted(
        edge_list,
        expected_vertex_order,
        expected_weights_when_visited,
        1
    );
}

void test7_weights() {
    const std::size_t edge_count = 2;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 1, 2 } } };

    const boost::array< mas_test_weight_type, edge_count > weights_list = { 2, 6 };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 1, 2 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 2, 6 };

    test_weighted(
        edge_list,
        weights_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

void test8_weights() {
    const std::size_t edge_count = 3;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 0, 2 }, { 1, 2 } } };

    const boost::array< mas_test_weight_type, edge_count > weights_list = { 2, 6, 7 };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 0, 2, 1 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 6, 9 };

    test_weighted(
        edge_list,
        weights_list,
        expected_vertex_order,
        expected_weights_when_visited
    );
}

void test9_weights_start_vertex() {
    const std::size_t edge_count = 3;
    const std::size_t vertices_count = 3;

    const boost::array< edge_t, edge_count > edge_list = { { { 0, 1 }, { 0, 2 }, { 1, 2 } } };

    const boost::array< mas_test_weight_type, edge_count > weights_list = { 2, 6, 7 };

    const boost::array< mas_test_vertex_descriptor, vertices_count > expected_vertex_order = { 1, 2, 0 };
    const boost::array< mas_test_weight_type, vertices_count > expected_weights_when_visited = { vertices_count+1, 7, 8 };

    test_weighted(
        edge_list,
        weights_list,
        expected_vertex_order,
        expected_weights_when_visited,
        1
    );
}

using cv_distances_map_type = boost::shared_array_property_map< weight_type,
    boost::property_map< undirected_graph, boost::vertex_index_t >::const_type >;
using cv_index_in_heap_type = std::vector< vertex_descriptor >::size_type;
using cv_indices_map_type = boost::shared_array_property_map< cv_index_in_heap_type,
    boost::property_map< undirected_graph, boost::vertex_index_t >::const_type >;
using cv_maxheap_type = boost::d_ary_heap_indirect< vertex_descriptor, 4, cv_indices_map_type,
    cv_distances_map_type, std::greater< weight_type > >;

// Build the keyed max priority queue MAS runs on: reach counts plus heap positions.
cv_maxheap_type make_weighted_maxheap(const undirected_graph& g)
{
    auto distances_map = boost::make_shared_array_property_map(num_vertices(g), weight_type(0), get(boost::vertex_index, g));
    auto indices_map = boost::make_shared_array_property_map(num_vertices(g), cv_index_in_heap_type(-1), get(boost::vertex_index, g));
    return cv_maxheap_type(distances_map, indices_map);
}

// Check invariants on a MAS run
void check_visit_order_invariants(
    const undirected_graph& g,
    const std::vector< vertex_descriptor >& order,
    const std::vector< weight_type >& reach)
{
    const std::size_t n = num_vertices(g);

    // 1) the order is a permutation of all vertices
    std::vector< vertex_descriptor > sorted = order;
    std::sort(sorted.begin(), sorted.end());
    for (std::size_t i = 0; i < sorted.size(); ++i)
        BOOST_TEST_EQ(sorted[i], static_cast< vertex_descriptor >(i));

    // 2) MAS adds n+1 to the start vertex key to force it first, so its recorded reach is n+1
    BOOST_TEST_EQ(reach[0], static_cast< weight_type >(n + 1));

    // only the start vertex is visited before the loop
    std::vector< bool > visited(n, false);
    visited[order[0]] = true;
    auto weight_map = get(boost::edge_weight, g);

    // 3) recompute each later vertex reach independently and check MAS agrees
    for (std::size_t i = 1; i < order.size(); ++i)
    {
        const vertex_descriptor u = order[i];
        // sum the weights of u's edges that lead back into the visited set
        weight_type expected = 0;
        boost::graph_traits< undirected_graph >::out_edge_iterator oi, oi_end;
        for (boost::tie(oi, oi_end) = out_edges(u, g); oi != oi_end; ++oi)
            if (visited[target(*oi, g)])
                expected += get(weight_map, *oi);

        BOOST_TEST_EQ(reach[i], expected);
        visited[u] = true;
    }
}

// Records every visitor event.
class recording_visitor : public boost::default_mas_visitor
{
public:
    recording_visitor(
        std::size_t& initialize_count,
        std::size_t& examine_count,
        std::vector< vertex_descriptor >& start_order,
        std::vector< vertex_descriptor >& finish_order
    )
    :
    initialize_count_(initialize_count),
    examine_count_(examine_count),
    start_order_(start_order),
    finish_order_(finish_order)
    {}

    void initialize_vertex(vertex_descriptor, const undirected_graph&) { ++initialize_count_; }
    void start_vertex(vertex_descriptor u, const undirected_graph&) { start_order_.push_back(u); }
    void examine_edge(edge_descriptor, const undirected_graph&) { ++examine_count_; }
    void finish_vertex(vertex_descriptor u, const undirected_graph&) { finish_order_.push_back(u); }

private:
    std::size_t& initialize_count_;
    std::size_t& examine_count_;
    std::vector< vertex_descriptor >& start_order_;
    std::vector< vertex_descriptor >& finish_order_;
};

// Cross-validation against the max-flow oracle
// for the last two visited vertices s (second-last) and t (last),
// the reach count of t equals its weighted degree and equals the min s-t cut.
void test_maxflow_crossvalidation()
{
    // check every curated graph
    for (const mas_sw_oracle::curated_graph& cg : mas_sw_oracle::curated_graphs())
    {
        const undirected_graph& g = cg.graph;
        BOOST_TEST(mas_sw_oracle::is_connected(g));

        // run MAS, recording visit order and reach counts
        cv_maxheap_type pq = make_weighted_maxheap(g);
        mas_test_visitor< undirected_graph, cv_maxheap_type > vis(pq);
        boost::maximum_adjacency_search(g, boost::weight_map(get(boost::edge_weight, g)).visitor(vis).max_priority_queue(pq));

        const std::vector< vertex_descriptor >& order = vis.vertex_visit_order();
        const std::vector< weight_type >& reach = vis.vertex_weights_when_visited();
        BOOST_TEST_EQ(order.size(), num_vertices(g));

        // take the last two visited vertices s and t
        const std::size_t last = order.size() - 1;
        const vertex_descriptor s = order[last - 1];
        const vertex_descriptor t = order[last];
        const weight_type reach_of_t = reach[last];

        // t's reach must equal its weighted degree and the min s-t cut
        BOOST_TEST_EQ(reach_of_t, mas_sw_oracle::weighted_degree(g, t));
        BOOST_TEST_EQ(reach_of_t, mas_sw_oracle::undirected_min_cut(g, s, t));

        // and the whole order must be a valid maximum adjacency ordering
        check_visit_order_invariants(g, order, reach);
    }
}

// Every visitor event fires the expected number of times
void test_visitor_events()
{
    const undirected_graph g = mas_sw_oracle::make_weighted_graph(6, { { 0, 1, 2 }, { 1, 2, 3 }, { 2, 0, 1 }, { 2, 3, 4 }, { 3, 4, 2 }, { 4, 5, 1 }, { 5, 3, 3 } });

    cv_maxheap_type pq = make_weighted_maxheap(g);
    std::size_t initialize_count = 0;
    std::size_t examine_count = 0;
    std::vector< vertex_descriptor > start_order;
    std::vector< vertex_descriptor > finish_order;
    recording_visitor vis(initialize_count, examine_count, start_order, finish_order);

    boost::maximum_adjacency_search(g, boost::weight_map(get(boost::edge_weight, g)).visitor(vis).max_priority_queue(pq));

    BOOST_TEST_EQ(initialize_count, static_cast< std::size_t >(num_vertices(g)));
    BOOST_TEST_EQ(examine_count, static_cast< std::size_t >(2 * num_edges(g)));
    BOOST_TEST_EQ(start_order.size(), static_cast< std::size_t >(num_vertices(g)));
    BOOST_TEST_EQ(finish_order.size(), static_cast< std::size_t >(num_vertices(g)));
    BOOST_TEST_ALL_EQ(start_order.begin(), start_order.end(), finish_order.begin(), finish_order.end());
}

// Precondition violations throw.
void test_exceptions()
{
    // a graph with fewer than two vertices is rejected
    undirected_graph too_small;
    add_vertex(too_small);
    BOOST_TEST_THROWS(boost::maximum_adjacency_search(too_small, boost::weight_map(get(boost::edge_weight, too_small))), boost::bad_graph);

    // a non-empty priority queue is rejected
    const undirected_graph g = mas_sw_oracle::make_weighted_graph(4, { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 } });
    cv_maxheap_type pq = make_weighted_maxheap(g);
    pq.push(0);
    BOOST_TEST_THROWS(boost::maximum_adjacency_search(g, boost::weight_map(get(boost::edge_weight, g)).max_priority_queue(pq)), std::invalid_argument);
}

#include <boost/graph/iteration_macros_undef.hpp>

int main(int argc, char* argv[])
{
    if (BOOST_TEST(argc == 2)) {
        test_dir = argv[1];
        test0();
        test1();
        test2_noweights();
        test3_noweights();
        test4_noweights();
        test5_Matula1993();
        test6_noweights_start_vertex();
        test7_weights();
        test8_weights();
        test9_weights_start_vertex();
        test_maxflow_crossvalidation();
        test_visitor_events();
        test_exceptions();
    }
    return boost::report_errors();
}
