// Copyright 2004 The Trustees of Indiana University.
// Copyright (c) 2026 Arnaud Becheler

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/sequential_vertex_coloring.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <cstddef>
#include <utility>
#include <vector>

using SimpleGraph = ::boost::adjacency_list< ::boost::listS, ::boost::vecS, ::boost::undirectedS >;
using BaseGraph = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, ::boost::undirectedS >;
using BaseVertex = ::boost::graph_traits< BaseGraph >::vertex_descriptor;

// Keeps the even numbered vertices, so vertices(g) is shorter than num_vertices(g).
struct keep_even
{
    bool operator()(BaseVertex v) const { return v % 2 == 0; }
};

using FilteredGraph = ::boost::filtered_graph< BaseGraph, ::boost::keep_all, keep_even >;
using size_type = ::boost::graph_traits< FilteredGraph >::vertices_size_type;

// Reports out of range queries instead of letting the algorithm read past the order.
struct counting_order
{
    const std::vector< BaseVertex >* order;
    std::size_t* out_of_range;

    BaseVertex operator()(size_type i) const
    {
        if (i < order->size())
            return (*order)[i];
        ++*out_of_range;
        return order->front();
    }
};

BaseGraph make_base_graph()
{
    using Edge = std::pair< std::size_t, std::size_t >;
    constexpr Edge edges[] = { Edge(0, 2), Edge(2, 4), Edge(1, 0), Edge(1, 2), Edge(3, 2), Edge(3, 4) };
    constexpr std::size_t m = sizeof(edges) / sizeof(Edge);
    return BaseGraph(edges, edges + m, 5);
}

std::vector< BaseVertex > vertex_order(const FilteredGraph& g)
{
    std::vector< BaseVertex > order;
    const auto vertex_range = ::boost::vertices(g);
    for (auto vi = vertex_range.first; vi != vertex_range.second; ++vi)
        order.push_back(*vi);
    return order;
}

constexpr size_type sentinel = 99;

void test_simple_graph()
{
    using vertex_descriptor = ::boost::graph_traits< SimpleGraph >::vertex_descriptor;
    using vertices_size_type = ::boost::graph_traits< SimpleGraph >::vertices_size_type;
    using Edge = std::pair< int, int >;
    enum nodes
    {
        A,
        B,
        C,
        D,
        E,
        n
    };
    constexpr Edge edge_array[] = { Edge(A, C), Edge(B, B), Edge(B, D), Edge(B, E), Edge(C, B), Edge(C, D), Edge(D, E), Edge(E, A), Edge(E, B) };
    constexpr std::size_t m = sizeof(edge_array) / sizeof(Edge);
    const SimpleGraph g(edge_array, edge_array + m, n);

    std::vector< vertices_size_type > color_vec(::boost::num_vertices(g));
    auto color_map = ::boost::make_iterator_property_map(color_vec.begin(), ::boost::get(::boost::vertex_index, g));
    const vertices_size_type num_colors = ::boost::sequential_vertex_coloring(g, color_map);

    BOOST_TEST_EQ(num_colors, 3);
    BOOST_TEST_EQ(::boost::get(color_map, static_cast< vertex_descriptor >(A)), 0);
    BOOST_TEST_EQ(::boost::get(color_map, static_cast< vertex_descriptor >(B)), 0);
    BOOST_TEST_EQ(::boost::get(color_map, static_cast< vertex_descriptor >(C)), 1);
    BOOST_TEST_EQ(::boost::get(color_map, static_cast< vertex_descriptor >(D)), 2);
    BOOST_TEST_EQ(::boost::get(color_map, static_cast< vertex_descriptor >(E)), 1);
}

// Regression for https://github.com/boostorg/graph/issues/627
void test_filtered_graph_order_bounds()
{
    const BaseGraph base = make_base_graph();
    const FilteredGraph g(base, ::boost::keep_all(), keep_even());
    const std::vector< BaseVertex > order = vertex_order(g);

    BOOST_TEST_EQ(order.size(), 3);
    BOOST_TEST_EQ(::boost::num_vertices(g), 5);

    std::size_t out_of_range = 0;
    const counting_order query = { &order, &out_of_range };
    auto order_map = ::boost::make_function_property_map< size_type >(query);

    std::vector< size_type > color_vec(::boost::num_vertices(g), sentinel);
    auto color_map = ::boost::make_iterator_property_map(color_vec.begin(), ::boost::get(::boost::vertex_index, g));
    const size_type num_colors = ::boost::sequential_vertex_coloring(g, order_map, color_map);

    BOOST_TEST_EQ(out_of_range, 0);
    BOOST_TEST_EQ(num_colors, 2);
    BOOST_TEST_EQ(color_vec[0], 0);
    BOOST_TEST_EQ(color_vec[2], 1);
    BOOST_TEST_EQ(color_vec[4], 0);
    BOOST_TEST_EQ(color_vec[1], sentinel);
    BOOST_TEST_EQ(color_vec[3], sentinel);
}

void test_filtered_graph_default_order()
{
    const BaseGraph base = make_base_graph();
    const FilteredGraph g(base, ::boost::keep_all(), keep_even());

    std::vector< size_type > color_vec(::boost::num_vertices(g), sentinel);
    auto color_map = ::boost::make_iterator_property_map(color_vec.begin(), ::boost::get(::boost::vertex_index, g));
    const size_type num_colors = ::boost::sequential_vertex_coloring(g, color_map);

    BOOST_TEST_EQ(num_colors, 2);
    BOOST_TEST_EQ(color_vec[0], 0);
    BOOST_TEST_EQ(color_vec[2], 1);
    BOOST_TEST_EQ(color_vec[4], 0);
    BOOST_TEST_EQ(color_vec[1], sentinel);
    BOOST_TEST_EQ(color_vec[3], sentinel);
}

int main()
{
    test_simple_graph();
    test_filtered_graph_order_bounds();
    test_filtered_graph_default_order();
    return ::boost::report_errors();
}
