// Copyright (C) 2006 Trustees of Indiana University
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/tuple/tuple.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <map>
#include <boost/graph/adj_list_serialize.hpp>
#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/serialization/tracking.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/string.hpp>
#include <boost/range/iterator_range.hpp>
#include <algorithm>
#include <cstddef>
#include <cstring>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

struct vertex_properties
{
    std::string name;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& BOOST_SERIALIZATION_NVP(name);
    }
};

struct edge_properties
{
    std::string name;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /*version*/)
    {
        ar& BOOST_SERIALIZATION_NVP(name);
    }
};

using namespace boost;

typedef adjacency_list< vecS, vecS, undirectedS, vertex_properties,
    edge_properties >
    Graph;

typedef graph_traits< Graph >::vertex_descriptor vd_type;

static_assert(boost::serialization::tracking_level< Graph >::value
        == boost::serialization::track_never,
    "adjacency_list tracking_level must be track_never");

typedef adjacency_list< vecS, vecS, undirectedS, vertex_properties >
    Graph_no_edge_property;

using ListGraph = boost::adjacency_list< boost::listS, boost::listS, boost::bidirectionalS, vertex_properties, edge_properties >;

using PlainGraph = boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS >;

using PropertyGraph = boost::adjacency_list< boost::vecS, boost::listS, boost::directedS, boost::property< boost::vertex_name_t, std::string >, boost::property< boost::edge_name_t, std::string, boost::property< boost::edge_weight_t, int > > >;

using EdgeRecord = std::tuple< std::size_t, std::size_t, std::string >;

void check_list_round_trip()
{
    ListGraph g;
    const auto a = boost::add_vertex(g);
    const auto b = boost::add_vertex(g);
    g[a].name = "A";
    g[b].name = "B";
    const auto ab = boost::add_edge(a, b, g).first;
    g[ab].name = "ab";
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        boost::archive::binary_oarchive oa(ss);
        oa << g;
    }
    ListGraph loaded;
    boost::archive::binary_iarchive ia(ss);
    ia >> loaded;
    constexpr std::size_t expected_num_vertices = 2;
    constexpr std::size_t expected_num_edges = 1;
    BOOST_TEST_EQ(boost::num_vertices(loaded), expected_num_vertices);
    BOOST_TEST_EQ(boost::num_edges(loaded), expected_num_edges);
    const auto first_vertex = *boost::vertices(loaded).first;
    const auto first_edge = *boost::edges(loaded).first;
    BOOST_TEST_EQ(loaded[first_vertex].name, "A");
    BOOST_TEST_EQ(loaded[first_edge].name, "ab");
}

std::size_t binary_archive_size(std::size_t num_edges)
{
    PlainGraph g(2);
    for (std::size_t i = 0; i < num_edges; ++i)
    {
        boost::add_edge(0, 1, g);
    }
    std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
    {
        boost::archive::binary_oarchive oa(ss);
        oa << g;
    }
    return ss.str().size();
}

// Each edge costs two 4 byte indices, as in archives from Boost 1.91 and earlier.
void check_binary_layout()
{
    constexpr std::size_t expected_edge_size = 2 * sizeof(unsigned int);
    const auto one_edge_size = binary_archive_size(1);
    const auto two_edges_size = binary_archive_size(2);
    BOOST_TEST_EQ(two_edges_size - one_edge_size, expected_edge_size);
}

// Gold archives hold A, B, C with edges ab1 and ab2 from A to B, bc from B to C and cc from C to C.
template < class G, class VertexTag, class EdgeTag >
void check_gold_graph(const G& g, VertexTag vertex_tag, EdgeTag edge_tag)
{
    using Vertex = typename boost::graph_traits< G >::vertex_descriptor;
    const std::vector< std::string > expected_vertex_names = { "A", "B", "C" };
    const std::vector< EdgeRecord > expected_edges = { EdgeRecord(0, 1, "ab1"), EdgeRecord(0, 1, "ab2"), EdgeRecord(1, 2, "bc"), EdgeRecord(2, 2, "cc") };
    const auto vertex_name_map = boost::get(vertex_tag, g);
    const auto edge_name_map = boost::get(edge_tag, g);
    std::unordered_map< Vertex, std::size_t > index_map;
    std::vector< std::string > vertex_names;
    for (const auto v : boost::make_iterator_range(boost::vertices(g)))
    {
        index_map[v] = vertex_names.size();
        vertex_names.push_back(boost::get(vertex_name_map, v));
    }
    std::vector< EdgeRecord > edges;
    for (const auto e : boost::make_iterator_range(boost::edges(g)))
    {
        const auto u = index_map[boost::source(e, g)];
        const auto v = index_map[boost::target(e, g)];
        edges.emplace_back(u, v, boost::get(edge_name_map, e));
    }
    std::sort(edges.begin(), edges.end());
    BOOST_TEST(vertex_names == expected_vertex_names);
    BOOST_TEST(edges == expected_edges);
}

void check_gold_weights(const PropertyGraph& g)
{
    const std::map< std::string, int > expected_weights = { { "ab1", 1 }, { "ab2", 2 }, { "bc", 3 }, { "cc", 4 } };
    const auto edge_name_map = boost::get(boost::edge_name, g);
    const auto weight_map = boost::get(boost::edge_weight, g);
    for (const auto e : boost::make_iterator_range(boost::edges(g)))
    {
        const auto& name = boost::get(edge_name_map, e);
        BOOST_TEST_EQ(boost::get(weight_map, e), expected_weights.at(name));
    }
}

void check_gold_plain(const PlainGraph& g)
{
    using PlainEdge = std::pair< std::size_t, std::size_t >;
    constexpr std::size_t expected_num_vertices = 3;
    const std::vector< PlainEdge > expected_edges = { PlainEdge(0, 1), PlainEdge(0, 1), PlainEdge(1, 2), PlainEdge(2, 2) };
    std::vector< PlainEdge > edges;
    for (const auto e : boost::make_iterator_range(boost::edges(g)))
    {
        edges.emplace_back(boost::source(e, g), boost::target(e, g));
    }
    std::sort(edges.begin(), edges.end());
    BOOST_TEST_EQ(boost::num_vertices(g), expected_num_vertices);
    BOOST_TEST(edges == expected_edges);
}

template < class IArchive >
void check_gold_archive(const std::string& path, std::ios::openmode mode)
{
    std::ifstream ifs(path, mode);
    BOOST_TEST(ifs.good());
    if (!ifs)
    {
        return;
    }
    IArchive ia(ifs);
    Graph vec_vec_undirected;
    ListGraph list_list_bidirectional;
    PropertyGraph vec_list_directed_property;
    PlainGraph vec_vec_directed_plain;
    ia >> boost::make_nvp("vec_vec_undirected", vec_vec_undirected);
    ia >> boost::make_nvp("list_list_bidirectional", list_list_bidirectional);
    ia >> boost::make_nvp("vec_list_directed_property", vec_list_directed_property);
    ia >> boost::make_nvp("vec_vec_directed_plain", vec_vec_directed_plain);
    check_gold_graph(vec_vec_undirected, &vertex_properties::name, &edge_properties::name);
    check_gold_graph(list_list_bidirectional, &vertex_properties::name, &edge_properties::name);
    check_gold_graph(vec_list_directed_property, boost::vertex_name, boost::edge_name);
    check_gold_weights(vec_list_directed_property);
    check_gold_plain(vec_vec_directed_plain);
}

// Native binary archives only load where sizes and byte order match the 64 bit little endian writer.
bool is_gold_binary_platform()
{
    constexpr unsigned int one = 1;
    unsigned char first_byte = 0;
    std::memcpy(&first_byte, &one, 1);
    return sizeof(int) == 4 && sizeof(long) == 8 && sizeof(std::size_t) == 8 && first_byte == 1;
}

void check_gold_archives(const std::string& directory)
{
    check_gold_archive< boost::archive::text_iarchive >(directory + "/boost_1_90.txt", std::ios::in);
    check_gold_archive< boost::archive::xml_iarchive >(directory + "/boost_1_90.xml", std::ios::in);
    if (is_gold_binary_platform())
    {
        check_gold_archive< boost::archive::binary_iarchive >(directory + "/boost_1_90.bin", std::ios::in | std::ios::binary);
    }
}

int main(int argc, char* argv[])
{
    check_list_round_trip();
    check_binary_layout();
    BOOST_TEST_GE(argc, 2);
    if (argc >= 2)
    {
        check_gold_archives(argv[1]);
    }

    {
        std::ofstream ofs("./kevin-bacon2.dat");
        archive::xml_oarchive oa(ofs);
        Graph g;
        vertex_properties vp;
        vp.name = "A";
        vd_type A = add_vertex(vp, g);
        vp.name = "B";
        vd_type B = add_vertex(vp, g);

        edge_properties ep;
        ep.name = "a";
        add_edge(A, B, ep, g);

        oa << BOOST_SERIALIZATION_NVP(g);

        Graph_no_edge_property g_n;
        oa << BOOST_SERIALIZATION_NVP(g_n);
    }

    {
        std::ifstream ifs("./kevin-bacon2.dat");
        archive::xml_iarchive ia(ifs);
        Graph g;
        ia >> BOOST_SERIALIZATION_NVP(g);

        if (!(g[*(vertices(g).first)].name == "A"))
            return -1;

        Graph_no_edge_property g_n;
        ia >> BOOST_SERIALIZATION_NVP(g_n);
    }
    return boost::report_errors();
}
