//=======================================================================
// Copyright 2007 Aaron Windsor
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Same planar pipeline as all_planar_input_files_test.cpp, with self loops
// and parallel edges added while each dimacs graph is read.

#define BOOST_FILESYSTEM_VERSION 3

#include <boost/filesystem.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/range/iterator_range.hpp>
#include <boost/core/lightweight_test.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/chrobak_payne_drawing.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/is_kuratowski_subgraph.hpp>
#include <boost/graph/is_straight_line_drawing.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_connected.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <boost/graph/planar_detail/add_edge_visitors.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <boost/graph/properties.hpp>

#include <cstddef>
#include <fstream>
#include <functional>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>

using graph_type = ::boost::adjacency_list< ::boost::listS, ::boost::vecS, ::boost::undirectedS, ::boost::property< ::boost::vertex_index_t, int >, ::boost::property< ::boost::edge_index_t, int > >;
using vertex_descriptor = ::boost::graph_traits< graph_type >::vertex_descriptor;
using edge_descriptor = ::boost::graph_traits< graph_type >::edge_descriptor;
using edge_index_map_type = ::boost::property_map< graph_type, ::boost::edge_index_t >::type;
using edge_updater_type = ::boost::edge_index_update_visitor< edge_index_map_type >;

// every fifth vertex gets one to six self loops
constexpr std::size_t vertex_stride = 5;
constexpr std::size_t max_loop_multiplicity = 6;

// every seventh edge gets one to five parallel copies
constexpr std::size_t edge_stride = 7;
constexpr std::size_t max_edge_multiplicity = 5;

struct grid_point
{
    std::size_t x;
    std::size_t y;
};

struct face_counter : ::boost::planar_face_traversal_visitor
{
    void begin_face() { ++count; }

    std::size_t count = 0;
};

graph_type read_dimacs(const std::string& filename)
{
    graph_type g;
    std::vector< vertex_descriptor > vertices_by_index;
    std::size_t num_edges_read = 0;
    std::size_t num_parallel_groups = 0;
    std::ifstream in(filename);
    std::string line;
    while (std::getline(in, line))
    {
        std::istringstream fields(line);
        std::string kind;
        fields >> kind;
        if (kind == "p")
        {
            std::string format;
            std::size_t vertex_count = 0;
            fields >> format >> vertex_count;
            g = graph_type(vertex_count);
            auto vertex_range = ::boost::vertices(g);
            vertices_by_index.assign(vertex_range.first, vertex_range.second);
            for (std::size_t index = 0; index < vertex_count; index += vertex_stride)
            {
                const auto num_loops = (index / vertex_stride) % max_loop_multiplicity + 1;
                const auto looped_vertex = vertices_by_index[index];
                for (std::size_t loop = 0; loop < num_loops; ++loop)
                {
                    ::boost::add_edge(looped_vertex, looped_vertex, g);
                }
            }
        }
        else if (kind == "e")
        {
            std::size_t source_index = 0;
            std::size_t target_index = 0;
            fields >> source_index >> target_index;
            const auto source_vertex = vertices_by_index[source_index];
            const auto target_vertex = vertices_by_index[target_index];
            ::boost::add_edge(source_vertex, target_vertex, g);
            if (num_edges_read % edge_stride == 0)
            {
                const auto num_copies = num_parallel_groups % max_edge_multiplicity + 1;
                for (std::size_t copy = 0; copy < num_copies; ++copy)
                {
                    ::boost::add_edge(source_vertex, target_vertex, g);
                }
                ++num_parallel_groups;
            }
            ++num_edges_read;
        }
    }
    return g;
}

void reset_edge_index(graph_type& g)
{
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    int next_index = 0;
    for (const auto edge : ::boost::make_iterator_range(::boost::edges(g)))
    {
        ::boost::put(edge_index_map, edge, next_index++);
    }
}

// a shared edge updater keeps the indices a permutation of the edges
bool has_dense_edge_index(const graph_type& g)
{
    const auto edge_count = ::boost::num_edges(g);
    std::vector< bool > seen(edge_count, false);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    for (const auto edge : ::boost::make_iterator_range(::boost::edges(g)))
    {
        const auto index = ::boost::get(edge_index_map, edge);
        if (index < 0 || static_cast< std::size_t >(index) >= edge_count || seen[static_cast< std::size_t >(index)])
        {
            return false;
        }
        seen[static_cast< std::size_t >(index)] = true;
    }
    return true;
}

void test_graph(const std::string& filename)
{
    auto g = read_dimacs(filename);
    reset_edge_index(g);

    auto vertex_index_map = ::boost::get(::boost::vertex_index, g);
    auto edge_index_map = ::boost::get(::boost::edge_index, g);
    const auto next_edge_index = static_cast< int >(::boost::num_edges(g));
    edge_updater_type edge_updater(edge_index_map, next_edge_index);
    auto edge_updater_ref = std::ref(edge_updater);

    ::boost::graph::make_connected(g, vertex_index_map, edge_updater_ref);
    BOOST_TEST(has_dense_edge_index(g));

    std::vector< std::vector< edge_descriptor > > embedding(::boost::num_vertices(g));
    auto embedding_map = ::boost::make_iterator_property_map(embedding.begin(), vertex_index_map);
    std::vector< edge_descriptor > kuratowski_edges;
    auto kuratowski_inserter = std::back_inserter(kuratowski_edges);
    const bool is_planar = ::boost::boyer_myrvold_planarity_test(::boost::boyer_myrvold_params::graph = g, ::boost::boyer_myrvold_params::embedding = embedding_map, ::boost::boyer_myrvold_params::kuratowski_subgraph = kuratowski_inserter);
    if (!is_planar)
    {
        BOOST_TEST(::boost::is_kuratowski_subgraph(g, kuratowski_edges.begin(), kuratowski_edges.end()));
        return;
    }

    ::boost::graph::make_biconnected_planar(g, embedding_map, edge_index_map, edge_updater_ref);
    BOOST_TEST(has_dense_edge_index(g));
    BOOST_TEST(::boost::boyer_myrvold_planarity_test(::boost::boyer_myrvold_params::graph = g, ::boost::boyer_myrvold_params::embedding = embedding_map));

    ::boost::graph::make_maximal_planar(g, embedding_map, vertex_index_map, edge_index_map, edge_updater_ref);
    BOOST_TEST(has_dense_edge_index(g));
    BOOST_TEST(::boost::boyer_myrvold_planarity_test(::boost::boyer_myrvold_params::graph = g, ::boost::boyer_myrvold_params::embedding = embedding_map));

    // Euler's formula on the final embedding
    face_counter counter;
    auto counter_ref = std::ref(counter);
    ::boost::graph::planar_face_traversal(g, embedding_map, counter_ref, edge_index_map);
    BOOST_TEST_EQ(::boost::num_vertices(g) + counter.count, ::boost::num_edges(g) + 2);

    std::vector< vertex_descriptor > ordering;
    auto ordering_inserter = std::back_inserter(ordering);
    ::boost::planar_canonical_ordering(g, embedding_map, ordering_inserter);
    BOOST_TEST_EQ(ordering.size(), ::boost::num_vertices(g));

    std::vector< grid_point > drawing(::boost::num_vertices(g));
    auto drawing_map = ::boost::make_iterator_property_map(drawing.begin(), vertex_index_map);
    ::boost::chrobak_payne_straight_line_drawing(g, embedding_map, ordering.begin(), ordering.end(), drawing_map);
    BOOST_TEST(::boost::is_straight_line_drawing(g, drawing_map));
}

int main(int argc, char* argv[])
{
    const std::string input_directory = argc > 1 ? argv[1] : "planar_input_graphs";
    const std::string dimacs_extension = ".dimacs";
    std::size_t num_files = 0;

    const ::boost::filesystem::directory_iterator directory_end;
    for (::boost::filesystem::directory_iterator entry(input_directory); entry != directory_end; ++entry)
    {
        if (entry->path().extension() != dimacs_extension)
        {
            continue;
        }
        test_graph(entry->path().string());
        ++num_files;
    }

    BOOST_TEST(num_files > 0u);
    return ::boost::report_errors();
}
