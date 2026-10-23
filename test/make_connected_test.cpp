//=======================================================================
// Copyright 2007 Aaron Windsor
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/make_connected.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>
#include <vector>

using vertex_property = ::boost::property< ::boost::vertex_index_t, int >;
using edge_property = ::boost::property< ::boost::edge_index_t, int >;
using vv_graph = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, ::boost::undirectedS, vertex_property, edge_property >;
using vl_graph = ::boost::adjacency_list< ::boost::vecS, ::boost::listS, ::boost::undirectedS, vertex_property, edge_property >;
using lv_graph = ::boost::adjacency_list< ::boost::listS, ::boost::vecS, ::boost::undirectedS, vertex_property, edge_property >;
using ll_graph = ::boost::adjacency_list< ::boost::listS, ::boost::listS, ::boost::undirectedS, vertex_property, edge_property >;

template < typename Graph > void reset_edge_index(Graph& g)
{
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    int next_index = 0;
    for (const auto edge : ::boost::make_iterator_range(::boost::edges(g)))
    {
        ::boost::put(edge_index_map, edge, next_index++);
    }
}

// listS vertices carry no index until one is assigned
struct reindex_vertices
{
    template < typename Graph > void operator()(Graph& g) const
    {
        auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
        int next_index = 0;
        for (const auto vertex : ::boost::make_iterator_range(::boost::vertices(g)))
        {
            ::boost::put(vertex_index_map, vertex, next_index++);
        }
    }
};

struct keep_vertex_index
{
    template < typename Graph > void operator()(Graph&) const {}
};

template < typename Graph > void make_disconnected_cycles(Graph& g, std::size_t num_cycles, std::size_t cycle_size)
{
    for (std::size_t cycle = 0; cycle < num_cycles; ++cycle)
    {
        const auto first_vertex = ::boost::add_vertex(g);
        auto previous_vertex = first_vertex;
        for (std::size_t position = 1; position < cycle_size; ++position)
        {
            const auto current_vertex = ::boost::add_vertex(g);
            ::boost::add_edge(previous_vertex, current_vertex, g);
            previous_vertex = current_vertex;
        }
        ::boost::add_edge(previous_vertex, first_vertex, g);
    }
}

template < typename Graph > int count_components(const Graph& g)
{
    std::vector< int > component(::boost::num_vertices(g));
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto component_map = ::boost::make_iterator_property_map(component.begin(), vertex_index_map);
    return ::boost::connected_components(g, component_map);
}

// adds the edge like the default visitor and counts the calls
struct counting_visitor
{
    template < typename Vertex, typename Graph > void visit_vertex_pair(Vertex u, Vertex v, Graph& g)
    {
        ::boost::add_edge(u, v, g);
        ++count;
    }

    std::size_t count = 0;
};

template < typename Graph, typename VertexIndexUpdater > void test_default_visitor(VertexIndexUpdater update_vertex_index, std::size_t num_cycles, std::size_t cycle_size)
{
    Graph g;
    make_disconnected_cycles(g, num_cycles, cycle_size);
    update_vertex_index(g);
    reset_edge_index(g);
    BOOST_TEST_EQ(count_components(g), static_cast< int >(num_cycles));

    ::boost::graph::make_connected(g);
    BOOST_TEST_EQ(count_components(g), 1);
    BOOST_TEST_EQ(::boost::num_edges(g), num_cycles * cycle_size + num_cycles - 1);

    // a connected graph gains no edge
    const auto connected_num_edges = ::boost::num_edges(g);
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    ::boost::graph::make_connected(g, vertex_index_map);
    BOOST_TEST_EQ(::boost::num_edges(g), connected_num_edges);
}

constexpr std::size_t small_num_cycles = 3;
constexpr std::size_t small_cycle_size = 4;

vv_graph make_disconnected_graph()
{
    vv_graph g;
    make_disconnected_cycles(g, small_num_cycles, small_cycle_size);
    reset_edge_index(g);
    return g;
}

void test_ref_visitor_is_updated()
{
    auto g = make_disconnected_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    counting_visitor visitor;
    auto visitor_ref = std::ref(visitor);

    ::boost::graph::make_connected(g, vertex_index_map, visitor_ref);

    BOOST_TEST_EQ(count_components(g), 1);
    BOOST_TEST_EQ(visitor.count, small_num_cycles - 1);
}

void test_ref_visitor_accumulates_across_calls()
{
    auto g = make_disconnected_graph();
    auto h = make_disconnected_graph();
    counting_visitor visitor;
    auto visitor_ref = std::ref(visitor);

    auto g_vertex_index_map = ::boost::get(::boost::vertex_index, g);
    ::boost::graph::make_connected(g, g_vertex_index_map, visitor_ref);
    auto h_vertex_index_map = ::boost::get(::boost::vertex_index, h);
    ::boost::graph::make_connected(h, h_vertex_index_map, visitor_ref);

    BOOST_TEST_EQ(visitor.count, 2 * (small_num_cycles - 1));
}

void test_named_visitor_is_copied()
{
    auto g = make_disconnected_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    counting_visitor visitor;

    ::boost::graph::make_connected(g, vertex_index_map, visitor);

    BOOST_TEST_EQ(count_components(g), 1);
    BOOST_TEST_EQ(visitor.count, 0u);
}

void test_const_visitor_is_copied()
{
    auto g = make_disconnected_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    const counting_visitor visitor{};

    ::boost::graph::make_connected(g, vertex_index_map, visitor);

    BOOST_TEST_EQ(count_components(g), 1);
    BOOST_TEST_EQ(visitor.count, 0u);
}

void test_temporary_visitor()
{
    auto g = make_disconnected_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);

    // passed inline on purpose to bind an rvalue
    ::boost::graph::make_connected(g, vertex_index_map, counting_visitor());

    BOOST_TEST_EQ(count_components(g), 1);
}

void test_connected_graph_is_not_visited()
{
    vv_graph g;
    make_disconnected_cycles(g, 1, small_cycle_size);
    reset_edge_index(g);
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    counting_visitor visitor;
    auto visitor_ref = std::ref(visitor);

    ::boost::graph::make_connected(g, vertex_index_map, visitor_ref);

    BOOST_TEST_EQ(visitor.count, 0u);
    BOOST_TEST_EQ(::boost::num_edges(g), small_cycle_size);
}

int main()
{
    test_default_visitor< vv_graph >(keep_vertex_index(), 10, 10);
    test_default_visitor< lv_graph >(keep_vertex_index(), 20, 20);
    test_default_visitor< vl_graph >(reindex_vertices(), 30, 30);
    test_default_visitor< ll_graph >(reindex_vertices(), 40, 40);

    test_ref_visitor_is_updated();
    test_ref_visitor_accumulates_across_calls();
    test_named_visitor_is_copied();
    test_const_visitor_is_copied();
    test_temporary_visitor();
    test_connected_graph_is_not_visited();

    return ::boost::report_errors();
}
