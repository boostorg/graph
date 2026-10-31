// Copyright 2026 Varun Sharma
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/nvp.hpp>
#include <boost/graph/csr_graph_serialize.hpp>
#include <boost/serialization/tracking.hpp>

#include <iterator>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

struct vertex_properties
{
    std::string name;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /* version */)
    {
        ar& boost::make_nvp("name", name);
    }
};

struct edge_properties
{
    edge_properties() : weight(0) { }
    explicit edge_properties(int value) : weight(value) { }

    int weight = 0;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /* version */)
    {
        ar& boost::make_nvp("weight", weight);
    }
};

struct graph_properties
{
    std::string name;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /* version */)
    {
        ar& boost::make_nvp("name", name);
    }
};

using CSRGraph = boost::compressed_sparse_row_graph< boost::directedS,
    vertex_properties, edge_properties, graph_properties >;
using PlainCSRGraph = boost::compressed_sparse_row_graph< boost::directedS >;
using BidirCSRGraph = boost::compressed_sparse_row_graph< boost::bidirectionalS,
    vertex_properties, edge_properties, graph_properties >;

static_assert(boost::serialization::tracking_level< CSRGraph >::value
        == boost::serialization::track_never,
    "compressed_sparse_row_graph tracking_level must be track_never");
static_assert(boost::serialization::tracking_level< PlainCSRGraph >::value
        == boost::serialization::track_never,
    "compressed_sparse_row_graph tracking_level must be track_never");

int main()
{
    using Vertex = CSRGraph::vertex_descriptor;
    using Edge = std::pair< Vertex, Vertex >;

    std::vector< Edge > endpoints { Edge(0, 1), Edge(0, 2), Edge(2, 1) };
    std::vector< edge_properties > edge_data { edge_properties { 7 },
        edge_properties { 11 }, edge_properties { 13 } };
    std::vector< Edge > plain_endpoints { Edge(0, 1), Edge(1, 2) };

    CSRGraph graph(boost::edges_are_sorted, endpoints.begin(), endpoints.end(),
        edge_data.begin(), 3, endpoints.size());
    BidirCSRGraph bidirectional_graph(boost::edges_are_sorted,
        endpoints.begin(), endpoints.end(), edge_data.begin(), 3,
        endpoints.size());
    PlainCSRGraph plain_graph(boost::edges_are_sorted, plain_endpoints.begin(),
        plain_endpoints.end(), 3);
    graph[0].name = "A";
    graph[1].name = "B";
    graph[2].name = "C";
    boost::get_property(graph, boost::graph_all_t()).name = "sample";
    bidirectional_graph[0].name = "A";
    bidirectional_graph[1].name = "B";
    bidirectional_graph[2].name = "C";
    boost::get_property(bidirectional_graph, boost::graph_all_t()).name
        = "sample";

    std::stringstream buffer;
    {
        boost::archive::xml_oarchive archive(buffer);
        archive << boost::make_nvp("graph", graph);
        archive << boost::make_nvp("bidirectional_graph", bidirectional_graph);
        archive << boost::make_nvp("plain_graph", plain_graph);
    }

    CSRGraph restored;
    BidirCSRGraph restored_bidirectional;
    PlainCSRGraph restored_plain_graph;
    {
        boost::archive::xml_iarchive archive(buffer);
        archive >> boost::make_nvp("graph", restored);
        archive >> boost::make_nvp(
            "bidirectional_graph", restored_bidirectional);
        archive >> boost::make_nvp("plain_graph", restored_plain_graph);
    }

    BOOST_TEST(boost::num_vertices(restored) == 3);
    BOOST_TEST(boost::num_edges(restored) == 3);
    BOOST_TEST(restored[0].name == "A");
    BOOST_TEST(restored[1].name == "B");
    BOOST_TEST(restored[2].name == "C");
    BOOST_TEST(
        boost::get_property(restored, boost::graph_all_t()).name == "sample");

    BOOST_TEST(boost::num_vertices(restored_bidirectional) == 3);
    BOOST_TEST(boost::num_edges(restored_bidirectional) == 3);
    BOOST_TEST(restored_bidirectional[0].name == "A");
    BOOST_TEST(restored_bidirectional[1].name == "B");
    BOOST_TEST(restored_bidirectional[2].name == "C");
    BOOST_TEST(boost::get_property(
                   restored_bidirectional, boost::graph_all_t()).name
        == "sample");

    const auto incoming = boost::in_edges(Vertex(1), restored_bidirectional);
    BOOST_TEST(boost::in_degree(Vertex(1), restored_bidirectional) == 2);
    BOOST_TEST(std::distance(incoming.first, incoming.second) == 2);

    bool found_0_to_1 = false;
    bool found_2_to_1 = false;
    for (auto it = incoming.first; it != incoming.second; ++it)
    {
        const Vertex source = boost::source(*it, restored_bidirectional);
        const Vertex target = boost::target(*it, restored_bidirectional);
        const int weight = restored_bidirectional[*it].weight;
        if (source == Vertex(0) && target == Vertex(1) && weight == 7)
            found_0_to_1 = true;
        else if (source == Vertex(2) && target == Vertex(1) && weight == 13)
            found_2_to_1 = true;
    }
    BOOST_TEST(found_0_to_1);
    BOOST_TEST(found_2_to_1);

    const auto e01 = boost::edge(Vertex(0), Vertex(1), restored);
    const auto e02 = boost::edge(Vertex(0), Vertex(2), restored);
    const auto e21 = boost::edge(Vertex(2), Vertex(1), restored);
    BOOST_TEST(e01.second);
    BOOST_TEST(e02.second);
    BOOST_TEST(e21.second);
    if (e01.second)
        BOOST_TEST(restored[e01.first].weight == 7);
    if (e02.second)
        BOOST_TEST(restored[e02.first].weight == 11);
    if (e21.second)
        BOOST_TEST(restored[e21.first].weight == 13);

    BOOST_TEST(boost::num_vertices(restored_plain_graph) == 3);
    BOOST_TEST(boost::num_edges(restored_plain_graph) == 2);
    BOOST_TEST(
        boost::edge(Vertex(0), Vertex(1), restored_plain_graph).second);
    BOOST_TEST(
        boost::edge(Vertex(1), Vertex(2), restored_plain_graph).second);

    return boost::report_errors();
}
