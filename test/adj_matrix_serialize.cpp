// Copyright 2026 Varun Sharma
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/archive/xml_iarchive.hpp>
#include <boost/archive/xml_oarchive.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/core/nvp.hpp>
#include <boost/graph/adj_matrix_serialize.hpp>
#include <boost/serialization/tracking.hpp>

#include <sstream>
#include <string>

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
    int weight;

    template < class Archive >
    void serialize(Archive& ar, const unsigned int /* version */)
    {
        ar& boost::make_nvp("weight", weight);
    }
};

using DirectedGraph = boost::adjacency_matrix< boost::directedS >;
using UndirectedGraph = boost::adjacency_matrix< boost::undirectedS,
    vertex_properties, edge_properties >;

static_assert(boost::serialization::tracking_level< DirectedGraph >::value
        == boost::serialization::track_never,
    "adjacency_matrix tracking_level must be track_never");
static_assert(boost::serialization::tracking_level< UndirectedGraph >::value
        == boost::serialization::track_never,
    "adjacency_matrix tracking_level must be track_never");

int main()
{
    DirectedGraph directed(3);
    boost::add_edge(0, 1, directed);
    boost::add_edge(1, 2, directed);

    UndirectedGraph undirected(3);
    undirected[0].name = "A";
    undirected[1].name = "B";
    undirected[2].name = "C";

    edge_properties first_edge = { 7 };
    edge_properties second_edge = { 11 };
    boost::add_edge(0, 1, first_edge, undirected);
    boost::add_edge(1, 2, second_edge, undirected);

    std::stringstream buffer;
    {
        boost::archive::xml_oarchive archive(buffer);
        archive << boost::make_nvp("directed_graph", directed);
        archive << boost::make_nvp("undirected_graph", undirected);
    }

    DirectedGraph restored_directed(0);
    UndirectedGraph restored_undirected(0);
    {
        boost::archive::xml_iarchive archive(buffer);
        archive >> boost::make_nvp("directed_graph", restored_directed);
        archive >> boost::make_nvp("undirected_graph", restored_undirected);
    }

    BOOST_TEST(boost::num_vertices(restored_directed) == 3);
    BOOST_TEST(boost::num_edges(restored_directed) == 2);
    BOOST_TEST(boost::edge(0, 1, restored_directed).second);
    BOOST_TEST(boost::edge(1, 2, restored_directed).second);
    BOOST_TEST(!boost::edge(1, 0, restored_directed).second);

    BOOST_TEST(boost::num_vertices(restored_undirected) == 3);
    BOOST_TEST(boost::num_edges(restored_undirected) == 2);
    BOOST_TEST(restored_undirected[0].name == "A");
    BOOST_TEST(restored_undirected[1].name == "B");
    BOOST_TEST(restored_undirected[2].name == "C");

    const auto first = boost::edge(0, 1, restored_undirected);
    const auto second = boost::edge(1, 2, restored_undirected);
    BOOST_TEST(first.second);
    BOOST_TEST(second.second);
    if (first.second)
        BOOST_TEST(restored_undirected[first.first].weight == 7);
    if (second.second)
        BOOST_TEST(restored_undirected[second.first].weight == 11);

    return boost::report_errors();
}
