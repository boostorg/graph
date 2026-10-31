// Copyright (C) 2026 Arnaud Becheler
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/core/lightweight_test.hpp>
#include <vector>

struct EdgeCentrality
{
    double centrality;
};

using Graph = boost::adjacency_list< boost::vecS, boost::vecS,
    boost::undirectedS, boost::no_property, EdgeCentrality >;

struct stop_after_first_removal
{
    int calls = 0;
    template < typename Edge, typename G >
    bool operator()(double, Edge, const G&)
    {
        return calls++ > 0;
    }
};

int main()
{
    Graph g(6);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(3, 4, g);
    boost::add_edge(4, 5, g);
    boost::add_edge(3, 5, g);
    boost::add_edge(2, 3, g);

    auto done = stop_after_first_removal();
    auto centrality_map = boost::get(&EdgeCentrality::centrality, g);
    boost::betweenness_centrality_clustering(g, done, centrality_map);

    auto bridge = boost::edge(2, 3, g);
    BOOST_TEST(!bridge.second);
    BOOST_TEST_EQ(boost::num_edges(g), 6u);

    std::vector< int > component(boost::num_vertices(g));
    auto component_map = &component[0];
    BOOST_TEST_EQ(boost::connected_components(g, component_map), 2);

    return boost::report_errors();
}
