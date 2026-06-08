//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_quality_functions.hpp>
#include <boost/core/lightweight_test.hpp>
#include <cmath>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using vertex_descriptor = boost::graph_traits<Graph>::vertex_descriptor;
using edge_descriptor = boost::graph_traits<Graph>::edge_descriptor;

bool approx_equal(double a, double b, double epsilon = 1e-6) {
    return std::abs(a - b) < epsilon;
}

// Test modularity with different graph partitions
void test_modularity_different_partitions() {
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    add_edge(3, 0, g);
    
    boost::static_property_map<double, edge_descriptor> weight_map(1.0);
    boost::newman_and_girvan f;

    // All in same community : Q = 0
    std::vector<vertex_descriptor> partition1 = {0, 0, 0, 0};
    auto pmap1 = boost::make_iterator_property_map(partition1.begin(), boost::get(boost::vertex_index, g));
    double Q1 = f.quality(g, pmap1, weight_map);
    BOOST_TEST(approx_equal(Q1, 0.0));

    // Two communities (0,1) and (2,3), symetric partition : Q = 0
    std::vector<vertex_descriptor> partition2 = {0, 0, 1, 1};
    auto pmap2 = boost::make_iterator_property_map(partition2.begin(), boost::get(boost::vertex_index, g));
    double Q2 = f.quality(g, pmap2, weight_map);
    BOOST_TEST(approx_equal(Q2, 0.0)); 

    // All separate communities, many inter-community edges: Q negative
    std::vector<vertex_descriptor> partition3 = {0, 1, 2, 3};
    auto pmap3 = boost::make_iterator_property_map(partition3.begin(), boost::get(boost::vertex_index, g));
    double Q3 = f.quality(g, pmap3, weight_map);
    BOOST_TEST(Q3 < 0.0);
}

// Test incremental state operations: remove, insert, gain
void test_state_operations() {
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    
    boost::static_property_map<double, edge_descriptor> weight_map(1.0);
    boost::newman_and_girvan f;

    // Initial partition: two communities {0,1} and {2,3}
    std::vector<vertex_descriptor> partition = {0, 0, 1, 1};
    auto pmap = boost::make_iterator_property_map(partition.begin(), boost::get(boost::vertex_index, g));
    
    // Create property maps
    boost::vector_property_map<double> k;
    std::map<vertex_descriptor, double> in_map, tot_map;
    auto in = boost::make_assoc_property_map(in_map);
    auto tot = boost::make_assoc_property_map(tot_map);
    double m;
    
    double Q_initial = f.quality(g, pmap, weight_map, k, in, tot, m);

    BOOST_TEST(approx_equal(Q_initial, 0.166667, 1e-5));

    // Test remove operation
    vertex_descriptor node_to_move = 1;
    vertex_descriptor old_comm = 0;
    double k_v = get(k, node_to_move); // degree of vertex 1 is 2
    double k_v_in_old = 1.0; // edge to vertex 0

    f.remove(in, tot, old_comm, k_v, k_v_in_old);
    
    // Community 0 originally had vertices {0,1} with tot = 1+2 = 3 (vertex 0 degree 1, vertex 1 degree 2)
    // After removing vertex 1 (degree 2), tot should be 3-2 = 1
    BOOST_TEST(approx_equal(get(tot, old_comm), 1.0));
    
    vertex_descriptor new_comm = 1;
    double k_v_in_new = 1.0;
    f.insert(in, tot, new_comm, k_v, k_v_in_new);
    
    // Original community 1 was {2,3} with tot = 2+1 = 3 (vertex 2 : degree 2, vertex 3 : degree 1)
    // After adding vertex 1 (degree 2), tot should be 3+2 = 5
    BOOST_TEST(approx_equal(get(tot, new_comm), 5.0));    
    
    // gain = k_v_in_new - (tot[new_comm] * k_v) / (2*m)
    //      = 1.0 - (5.0 * 2.0) / (2 * 3.0) = 1.0 - 10.0/6.0 ~= -0.667
    double gain = f.gain(tot, m, new_comm, k_v_in_new, k_v);
    BOOST_TEST(approx_equal(gain, -0.666667, 1e-5));
}

// Test weighted graph modularity
void test_modularity_weighted() {
    Graph g(3);
    auto e1 = add_edge(0, 1, g).first;
    auto e2 = add_edge(1, 2, g).first;
    auto e3 = add_edge(0, 2, g).first;
    
    std::map<edge_descriptor, double> weights;
    weights[e1] = 1.0;
    weights[e2] = 2.0;
    weights[e3] = 1.0;
    
    auto weight_map = boost::make_assoc_property_map(weights);
    std::vector<vertex_descriptor> partition = {0, 0, 0};
    auto pmap = boost::make_iterator_property_map(partition.begin(), boost::get(boost::vertex_index, g));
    
    // Create property maps
    boost::vector_property_map<double> k;
    std::map<vertex_descriptor, double> in_map, tot_map;
    auto in = boost::make_assoc_property_map(in_map);
    auto tot = boost::make_assoc_property_map(tot_map);
    double m;
    
    boost::newman_and_girvan f;
    double Q = f.quality(g, pmap, weight_map, k, in, tot, m);
    
    // Complete graph in one community : Q=0
    BOOST_TEST(approx_equal(Q, 0.0));
    BOOST_TEST(approx_equal(m, 4.0));
    BOOST_TEST(approx_equal(get(k, 0), 2.0));
    BOOST_TEST(approx_equal(get(k, 1), 3.0));
    BOOST_TEST(approx_equal(get(k, 2), 3.0));
}

// Test modularity with self-loop
// 2 vertices, each with self-loop weight 1, connected by edge weight 2, one community
// Manual calculation (self-loops count twice):
//   Self-loops count twice in degree: k[0] = 2*1 + 2 = 4, k[1] = 2*1 + 2 = 4
//   m = (k[0] + k[1]) / 2 = 8/2 = 4, so 2m = 8
//   Community 0: tot = 8, in = 2*1 + 2*2 + 2*1 = 8
//   Q = (1/8) * (8 - 64/8) = 0
void test_modularity_with_selfloop() {
    Graph g(2);
    auto e00 = add_edge(0, 0, g).first;
    auto e01 = add_edge(0, 1, g).first;
    auto e11 = add_edge(1, 1, g).first;
    
    std::map<edge_descriptor, double> weights;
    weights[e00] = 1.0;
    weights[e01] = 2.0;
    weights[e11] = 1.0;
    
    auto weight_map = boost::make_assoc_property_map(weights);
    
    // All in same community
    std::vector<vertex_descriptor> partition = {0, 0};
    auto pmap = boost::make_iterator_property_map(partition.begin(), boost::get(boost::vertex_index, g));
    
    // Create property maps
    boost::vector_property_map<double> k;
    std::map<vertex_descriptor, double> in_map, tot_map;
    auto in = boost::make_assoc_property_map(in_map);
    auto tot = boost::make_assoc_property_map(tot_map);
    double m;
    
    boost::newman_and_girvan f;
    double Q = f.quality(g, pmap, weight_map, k, in, tot, m);
    
    double expected_Q = 0.0;
    
    BOOST_TEST(approx_equal(Q, expected_Q));
    BOOST_TEST(approx_equal(m, 4.0));
    BOOST_TEST(approx_equal(get(k, 0), 4.0));
    BOOST_TEST(approx_equal(get(k, 1), 4.0));
    BOOST_TEST(approx_equal(get(tot, 0), 8.0));
    BOOST_TEST(approx_equal(get(in, 0), 8.0));
}

int main() {
    test_modularity_different_partitions();
    test_state_operations();
    test_modularity_weighted();
    test_modularity_with_selfloop();
    return boost::report_errors();
}
