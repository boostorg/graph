//=======================================================================
// Copyright 2026 Matyas W Egyhazy
// Copyright (C) 2026 Arnaud Becheler
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/geometric_graph_generator.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>

#include <cstddef>
#include <iostream>
#include <random>
#include <vector>

namespace
{

using Graph = ::boost::adjacency_matrix< ::boost::undirectedS, ::boost::no_property, ::boost::property< ::boost::edge_weight_t, double > >;
using Point = ::boost::simple_point< double >;
using Edge = ::boost::graph_traits< Graph >::edge_descriptor;

} // end anonymous namespace

int main()
{
    constexpr std::size_t num_points = 20;

    Graph g(num_points);
    std::mt19937 gen(42);
    std::uniform_real_distribution< double > coordinates(0.0, 500.0);
    auto weight_map = ::boost::get(::boost::edge_weight, g);
    auto index_map = ::boost::get(::boost::vertex_index, g);
    const ::boost::graph::euclidean_distance euclidean {};

    ::boost::graph::make_random_geometric_graph< Point >(g, num_points, coordinates, coordinates, weight_map, index_map, gen, euclidean);

    const auto vertex_count = ::boost::num_vertices(g);
    const auto edge_count = ::boost::num_edges(g);
    std::cout << "Complete geometric graph: " << vertex_count << " vertices, " << edge_count << " edges\n";

    std::vector< Edge > mst;
    auto mst_out = std::back_inserter(mst);
    ::boost::kruskal_minimum_spanning_tree(g, mst_out);

    double total = 0.0;

    for (const Edge& e : mst)
        total += ::boost::get(weight_map, e);

    std::cout << "Minimum spanning tree: " << mst.size() << " edges, total weight " << total << "\n";

    return 0;
}
