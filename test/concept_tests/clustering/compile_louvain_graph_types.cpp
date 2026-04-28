//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Verify that louvain_clustering works with different graph data structures.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/louvain_clustering.hpp>
#include <boost/core/lightweight_test.hpp>
#include <random>
#include <set>

// Helper: build a triangle (3 vertices, 3 edges), run louvain_clustering, check basic postconditions.
template <typename Graph, typename WeightMap>
void run_triangle_test(Graph& g, const WeightMap& wmap)
{
    using vertex_t = typename boost::graph_traits<Graph>::vertex_descriptor;

    std::vector<vertex_t> clusters(boost::num_vertices(g));
    auto cmap = boost::make_iterator_property_map(clusters.begin(), boost::get(boost::vertex_index, g));

    std::mt19937 rng(42);
    double Q = boost::louvain_clustering(g, cmap, wmap, rng);

    // Modularity must be in [0, 1] for a connected undirected graph
    BOOST_TEST(Q >= 0.0);
    BOOST_TEST(Q <= 1.0);

    // Every vertex must be assigned a community
    std::set<vertex_t> communities(clusters.begin(), clusters.end());
    BOOST_TEST(communities.size() >= 1u);
}

// adjacency_list<vecS, vecS, undirectedS> 
void test_vecS_vecS()
{
    using Graph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double> >;

    Graph g(3);
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);

    run_triangle_test(g, boost::get(boost::edge_weight, g));
}

// adjacency_list<listS, vecS, undirectedS> 
void test_listS_vecS()
{
    using Graph = boost::adjacency_list<
        boost::listS, boost::vecS, boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double> >;

    Graph g(3);
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);

    run_triangle_test(g, boost::get(boost::edge_weight, g));
}

// adjacency_list<setS, vecS, undirectedS> 
void test_setS_vecS()
{
    using Graph = boost::adjacency_list<
        boost::setS, boost::vecS, boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double> >;

    Graph g(3);
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);

    run_triangle_test(g, boost::get(boost::edge_weight, g));
}

//  adjacency_matrix<undirectedS> 
void test_adjacency_matrix()
{
    using Graph = boost::adjacency_matrix<
        boost::undirectedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double> >;

    Graph g(3);
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);

    run_triangle_test(g, boost::get(boost::edge_weight, g));
}

int main()
{
    test_vecS_vecS();
    test_listS_vecS();
    test_setS_vecS();
    test_adjacency_matrix();
    return boost::report_errors();
}