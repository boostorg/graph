//============================================================================
// Copyright 2019 Cromwell D. Enage
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//============================================================================

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/metric_tsp_approx.hpp>
#include <boost/core/lightweight_test.hpp>

#include <vector>
#include <string>

// The example is from:
// http://www.cs.tufts.edu/~cowen/advanced/2002/adv-lect3.pdf
// The program should match the answer given by Christofides' Algorithm:
// ABCSEM (+A for this implementation)

int main(int argc, char* argv[])
{
    std::vector<std::string> string_vec;

    string_vec.push_back("Arlington");
    string_vec.push_back("Belmont");
    string_vec.push_back("Cambridge");
    string_vec.push_back("Everett");
    string_vec.push_back("Medford");
    string_vec.push_back("Somerville");

    typedef boost::adjacency_matrix<
        boost::undirectedS, boost::no_property,
        boost::property<boost::edge_weight_t, double>
    > Graph;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef boost::property_map<Graph, boost::edge_weight_t>::type WeightMap;

    Graph g(string_vec.size());
    WeightMap e_weight_map = get(boost::edge_weight, g);
    Edge e;
    bool b;

    boost::tie(e, b) = add_edge(vertex(0, g), vertex(1, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 10.0);
    boost::tie(e, b) = add_edge(vertex(0, g), vertex(2, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 15.0);
    boost::tie(e, b) = add_edge(vertex(0, g), vertex(3, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 14.0);
    boost::tie(e, b) = add_edge(vertex(0, g), vertex(4, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 11.0);
    boost::tie(e, b) = add_edge(vertex(0, g), vertex(5, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 10.0);
    boost::tie(e, b) = add_edge(vertex(1, g), vertex(2, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 8.0);
    boost::tie(e, b) = add_edge(vertex(1, g), vertex(3, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 13.0);
    boost::tie(e, b) = add_edge(vertex(1, g), vertex(4, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 15.0);
    boost::tie(e, b) = add_edge(vertex(1, g), vertex(5, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 9.0);
    boost::tie(e, b) = add_edge(vertex(2, g), vertex(3, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 11.0);
    boost::tie(e, b) = add_edge(vertex(2, g), vertex(4, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 16.0);
    boost::tie(e, b) = add_edge(vertex(2, g), vertex(5, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 10.0);
    boost::tie(e, b) = add_edge(vertex(3, g), vertex(4, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 9.0);
    boost::tie(e, b) = add_edge(vertex(3, g), vertex(5, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 6.0);
    boost::tie(e, b) = add_edge(vertex(4, g), vertex(5, g), g);
    BOOST_TEST(b);
    put(e_weight_map, e, 9.0);

    typedef boost::property_map<Graph, boost::vertex_index_t>::type VIndexMap;
    VIndexMap v_index_map = get(boost::vertex_index, g);
    std::vector<Vertex> c;

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    boost::metric_tsp_approx(
        g, vertex(0, g), std::back_inserter(c)
    );
#elif defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    boost::metric_tsp_approx_tour(
        g,
        boost::graph::keywords::_result = std::back_inserter(c),
        boost::graph::keywords::_root_vertex = vertex(0, g)
    );
#else
    boost::metric_tsp_approx_tour_from_vertex(
        g, vertex(0, g), std::back_inserter(c)
    );
#endif

    BOOST_TEST_EQ(0, get(v_index_map, c[0]));
    BOOST_TEST_EQ(1, get(v_index_map, c[1]));
    BOOST_TEST_EQ(2, get(v_index_map, c[2]));
    BOOST_TEST_EQ(5, get(v_index_map, c[3]));
    BOOST_TEST_EQ(3, get(v_index_map, c[4]));
    BOOST_TEST_EQ(4, get(v_index_map, c[5]));
    BOOST_TEST_EQ(0, get(v_index_map, c[6]));

    return boost::report_errors();
}

