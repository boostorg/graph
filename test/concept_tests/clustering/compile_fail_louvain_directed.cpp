//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// This file must FAIL to compile.
// louvain_clustering requires an undirected graph until implementation 
// supports a directed version of modularity computation.
//
// A directed adjacency_list should be rejected by the static_assert.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_clustering.hpp>
#include <random>

int main()
{
    using Graph = boost::adjacency_list<
        boost::vecS,
        boost::vecS, 
        boost::directedS,
        boost::no_property,
        boost::property<boost::edge_weight_t, double>
        >;

    Graph g(3);
    boost::add_edge(0, 1, 1.0, g);
    boost::add_edge(1, 2, 1.0, g);
    boost::add_edge(0, 2, 1.0, g);

    boost::vector_property_map<boost::graph_traits<Graph>::vertex_descriptor> clusters;
    auto weight_map = boost::get(boost::edge_weight, g);

    std::mt19937 rng(42);
    boost::louvain_clustering(g, clusters, weight_map, rng);
}
