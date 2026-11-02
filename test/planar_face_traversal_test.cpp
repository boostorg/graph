//=======================================================================
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>

using graph_type = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, ::boost::undirectedS, ::boost::no_property, ::boost::property< ::boost::edge_index_t, int > >;
using edge_descriptor = ::boost::graph_traits< graph_type >::edge_descriptor;
using embedding_storage = std::vector< std::vector< edge_descriptor > >;
using edge_list = std::vector< std::pair< std::size_t, std::size_t > >;

// counts faces and edge visits and checks the events nest correctly
struct event_recorder : ::boost::planar_face_traversal_visitor
{
    void begin_traversal()
    {
        well_formed = well_formed && !in_traversal;
        in_traversal = true;
    }

    void begin_face()
    {
        well_formed = well_formed && in_traversal && !in_face;
        in_face = true;
        ++faces;
    }

    template < typename Edge > void next_edge(Edge)
    {
        well_formed = well_formed && in_face;
        ++edge_visits;
    }

    void end_face()
    {
        well_formed = well_formed && in_face;
        in_face = false;
    }

    void end_traversal()
    {
        well_formed = well_formed && in_traversal && !in_face;
        in_traversal = false;
    }

    std::size_t faces = 0;
    std::size_t edge_visits = 0;
    bool in_traversal = false;
    bool in_face = false;
    bool well_formed = true;
};

graph_type make_graph(std::size_t vertex_count, const edge_list& edges)
{
    graph_type g(vertex_count);
    for (const auto& endpoints : edges)
    {
        ::boost::add_edge(endpoints.first, endpoints.second, g);
    }
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    int next_index = 0;
    for (const auto edge : ::boost::make_iterator_range(::boost::edges(g)))
    {
        ::boost::put(edge_index_map, edge, next_index++);
    }
    return g;
}

embedding_storage planar_embedding(const graph_type& g)
{
    embedding_storage embedding(::boost::num_vertices(g));
    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto embedding_map = ::boost::make_iterator_property_map(embedding.begin(), vertex_index_map);
    const bool is_planar = ::boost::boyer_myrvold_planarity_test(::boost::boyer_myrvold_params::graph = g, ::boost::boyer_myrvold_params::embedding = embedding_map);
    BOOST_TEST(is_planar);
    return embedding;
}

const edge_list cycle_edges = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 0 } };
constexpr std::size_t cycle_faces = 2;

struct planar_case
{
    std::size_t vertex_count;
    edge_list edges;
    std::size_t faces;
};

void test_curated_graphs()
{
    const std::vector< planar_case > cases = {
        // a tree has a single face
        { 5, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 } }, 1 },
        { 5, cycle_edges, cycle_faces },
        // every vertex has degree three
        { 4, { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 }, { 2, 3 } }, 4 },
        // the isolated vertex is never visited
        { 6, cycle_edges, cycle_faces },
        // the loop adds its own inner face
        { 5, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 0 }, { 0, 0 } }, 3 },
        // the doubled edge encloses a face
        { 5, { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 4 }, { 4, 0 }, { 0, 1 } }, 3 },
    };

    for (const auto& planar : cases)
    {
        const auto g = make_graph(planar.vertex_count, planar.edges);
        auto embedding = planar_embedding(g);
        auto embedding_map = embedding.data();
        event_recorder recorder;
        auto recorder_ref = std::ref(recorder);

        ::boost::graph::planar_face_traversal(g, embedding_map, recorder_ref);

        BOOST_TEST(recorder.well_formed);
        BOOST_TEST(!recorder.in_traversal);
        BOOST_TEST_EQ(recorder.faces, planar.faces);
        BOOST_TEST_EQ(recorder.edge_visits, 2 * ::boost::num_edges(g));
    }
}

void test_edge_index_overload()
{
    const auto g = make_graph(5, cycle_edges);
    auto embedding = planar_embedding(g);
    auto embedding_map = embedding.data();
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    event_recorder recorder;
    auto recorder_ref = std::ref(recorder);

    ::boost::graph::planar_face_traversal(g, embedding_map, recorder_ref, edge_index_map);

    BOOST_TEST_EQ(recorder.faces, cycle_faces);
}

void test_ref_visitor_accumulates()
{
    const auto g = make_graph(5, cycle_edges);
    auto embedding = planar_embedding(g);
    auto embedding_map = embedding.data();
    event_recorder recorder;
    auto recorder_ref = std::ref(recorder);

    ::boost::graph::planar_face_traversal(g, embedding_map, recorder_ref);
    ::boost::graph::planar_face_traversal(g, embedding_map, recorder_ref);

    BOOST_TEST_EQ(recorder.faces, 2 * cycle_faces);
}

void test_named_visitor_is_copied()
{
    const auto g = make_graph(5, cycle_edges);
    auto embedding = planar_embedding(g);
    auto embedding_map = embedding.data();
    event_recorder recorder;

    ::boost::graph::planar_face_traversal(g, embedding_map, recorder);

    BOOST_TEST_EQ(recorder.faces, 0u);
}

void test_const_visitor_is_copied()
{
    const auto g = make_graph(5, cycle_edges);
    auto embedding = planar_embedding(g);
    auto embedding_map = embedding.data();
    const event_recorder recorder{};

    ::boost::graph::planar_face_traversal(g, embedding_map, recorder);

    BOOST_TEST_EQ(recorder.faces, 0u);
}

void test_temporary_visitor()
{
    const auto g = make_graph(5, cycle_edges);
    auto embedding = planar_embedding(g);
    auto embedding_map = embedding.data();

    // passed inline on purpose to bind an rvalue
    ::boost::graph::planar_face_traversal(g, embedding_map, event_recorder());
}

int main()
{
    test_curated_graphs();
    test_edge_index_overload();
    test_ref_visitor_accumulates();
    test_named_visitor_is_copied();
    test_const_visitor_is_copied();
    test_temporary_visitor();

    return ::boost::report_errors();
}
