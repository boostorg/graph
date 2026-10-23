//=======================================================================
// Copyright 2007 Aaron Windsor
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_maximal_planar.hpp>
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
using ss_graph = ::boost::adjacency_list< ::boost::setS, ::boost::setS, ::boost::undirectedS, vertex_property, edge_property >;

template < typename Graph > void reset_edge_index(Graph& g)
{
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    int next_index = 0;
    for (const auto edge : ::boost::make_iterator_range(::boost::edges(g)))
    {
        ::boost::put(edge_index_map, edge, next_index++);
    }
}

// listS and setS vertices carry no index until one is assigned
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

// a cycle is biconnected and planar, the smallest valid input
template < typename Graph > void make_cycle(Graph& g, std::size_t size)
{
    const auto first_vertex = ::boost::add_vertex(g);
    auto previous_vertex = first_vertex;
    for (std::size_t position = 1; position < size; ++position)
    {
        const auto current_vertex = ::boost::add_vertex(g);
        ::boost::add_edge(previous_vertex, current_vertex, g);
        previous_vertex = current_vertex;
    }
    ::boost::add_edge(previous_vertex, first_vertex, g);
}

// every vertex of a cycle has two edges, so any rotation is an embedding
template < typename Graph > std::vector< std::vector< typename ::boost::graph_traits< Graph >::edge_descriptor > > out_edge_embedding(const Graph& g)
{
    using edge_descriptor = typename ::boost::graph_traits< Graph >::edge_descriptor;
    std::vector< std::vector< edge_descriptor > > embedding(::boost::num_vertices(g));
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    for (const auto vertex : ::boost::make_iterator_range(::boost::vertices(g)))
    {
        const auto index = static_cast< std::size_t >(::boost::get(vertex_index_map, vertex));
        auto out_edge_range = ::boost::out_edges(vertex, g);
        embedding[index].assign(out_edge_range.first, out_edge_range.second);
    }
    return embedding;
}

// planar with 3n minus 6 edges is exactly maximal planar
template < typename Graph > void check_maximal_planar(Graph& g)
{
    reset_edge_index(g);
    BOOST_TEST_EQ(::boost::num_edges(g), 3 * ::boost::num_vertices(g) - 6);
    BOOST_TEST(::boost::boyer_myrvold_planarity_test(g));
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

template < typename Graph, typename VertexIndexUpdater > void test_default_visitor(VertexIndexUpdater update_vertex_index, std::size_t size)
{
    Graph g;
    make_cycle(g, size);
    update_vertex_index(g);
    reset_edge_index(g);
    BOOST_TEST(::boost::boyer_myrvold_planarity_test(g));

    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);
    ::boost::graph::make_maximal_planar(g, embedding_map);

    check_maximal_planar(g);
}

constexpr std::size_t cycle_size = 6;

vv_graph make_cycle_graph()
{
    vv_graph g;
    make_cycle(g, cycle_size);
    reset_edge_index(g);
    return g;
}

void test_vertex_index_overload()
{
    auto g = make_cycle_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map);

    check_maximal_planar(g);
}

void test_edge_index_overload()
{
    auto g = make_cycle_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map);

    check_maximal_planar(g);
}

void test_ref_visitor_is_updated()
{
    auto g = make_cycle_graph();
    const auto initial_num_edges = ::boost::num_edges(g);
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);
    counting_visitor visitor;
    auto visitor_ref = std::ref(visitor);

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map, visitor_ref);

    check_maximal_planar(g);
    BOOST_TEST(visitor.count > 0u);
    BOOST_TEST_EQ(visitor.count, ::boost::num_edges(g) - initial_num_edges);
}

void test_named_visitor_is_copied()
{
    auto g = make_cycle_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);
    counting_visitor visitor;

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map, visitor);

    check_maximal_planar(g);
    BOOST_TEST_EQ(visitor.count, 0u);
}

void test_const_visitor_is_copied()
{
    auto g = make_cycle_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);
    const counting_visitor visitor{};

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map, visitor);

    check_maximal_planar(g);
    BOOST_TEST_EQ(visitor.count, 0u);
}

void test_temporary_visitor()
{
    auto g = make_cycle_graph();
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    auto embedding_storage = out_edge_embedding(g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding_storage.begin(), vertex_index_map);

    // passed inline on purpose to bind an rvalue
    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map, counting_visitor());

    check_maximal_planar(g);
}

int main()
{
    test_default_visitor< vv_graph >(keep_vertex_index(), 10);
    test_default_visitor< vv_graph >(keep_vertex_index(), 50);
    test_default_visitor< vl_graph >(reindex_vertices(), 3);
    test_default_visitor< vl_graph >(reindex_vertices(), 30);
    test_default_visitor< lv_graph >(keep_vertex_index(), 15);
    test_default_visitor< lv_graph >(keep_vertex_index(), 45);
    test_default_visitor< ll_graph >(reindex_vertices(), 8);
    test_default_visitor< ll_graph >(reindex_vertices(), 19);
    test_default_visitor< ss_graph >(reindex_vertices(), 13);
    test_default_visitor< ss_graph >(reindex_vertices(), 20);

    test_vertex_index_overload();
    test_edge_index_overload();
    test_ref_visitor_is_updated();
    test_named_visitor_is_copied();
    test_const_visitor_is_copied();
    test_temporary_visitor();

    return ::boost::report_errors();
}
