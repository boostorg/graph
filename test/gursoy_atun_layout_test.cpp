// Copyright 2004 The Trustees of Indiana University.
// Copyright (c) 2026 Arnaud Becheler

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Jeremiah Willcock
//           Douglas Gregor
//           Andrew Lumsdaine

#include <boost/graph/gursoy_atun_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <cmath>
#include <map>
#include <random>
#include <vector>

using graph_type = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, boost::property< boost::edge_weight_t, double > >;
using vertex_index_map = boost::property_map< graph_type, boost::vertex_index_t >::type;
using topology = boost::heart_topology<>;
using point = topology::point_type;
using position_map = boost::iterator_property_map< std::vector< point >::iterator, vertex_index_map, point, point& >;

// Builds the 20x20 weighted grid Gursoy and Atun used
void build_grid(graph_type& g)
{
    using vertex_descriptor = boost::graph_traits< graph_type >::vertex_descriptor;
    constexpr int grid_size = 20;
    std::map< int, std::map< int, vertex_descriptor > > verts;
    std::mt19937 engine(42);
    std::uniform_real_distribution< double > dist(0.0, 1.0);
    auto random_edge_weight = [&] { return dist(engine); };

    for (int i = 0; i < grid_size; ++i)
        for (int j = 0; j < grid_size; ++j)
            verts[i][j] = boost::add_vertex(g);

    for (int i = 0; i < grid_size; ++i)
    {
        for (int j = 0; j < grid_size; ++j)
        {
            if (i != 0)
                boost::add_edge(verts[i][j], verts[i - 1][j], random_edge_weight(), g);
            if (j != 0)
                boost::add_edge(verts[i][j], verts[i][j - 1], random_edge_weight(), g);
        }
    }
}

// Positions are heuristic, so we assert invariants instead of exact coordinates
// every point finite, inside the bounds, not all collapsed in one spot.
bool layout_is_valid(const std::vector< point >& pos)
{
    constexpr double eps = 1e-6;
    constexpr double y_min = -2000.0;
    // The heart's two circles bulge past the square to x = +-500(1 + sqrt2)
    // and up to y = 500(sqrt2 - 1) and the square's tip reaches y = -2000.
    const double x_max = 500.0 * (1.0 + std::sqrt(2.0));
    const double x_min = -x_max;
    const double y_max = 500.0 * (std::sqrt(2.0) - 1.0);
    double min_x = pos[0][0], max_x = pos[0][0], min_y = pos[0][1], max_y = pos[0][1];

    for (const point& p : pos)
    {
        if (!std::isfinite(p[0]) || !std::isfinite(p[1]))
            return false;
        if (p[0] < x_min - eps || p[0] > x_max + eps)
            return false;
        if (p[1] < y_min - eps || p[1] > y_max + eps)
            return false;
        min_x = std::min(min_x, p[0]);
        max_x = std::max(max_x, p[0]);
        min_y = std::min(min_y, p[1]);
        max_y = std::max(max_y, p[1]);
    }
    constexpr double spread_threshold = 1.0;
    return (max_x - min_x) > spread_threshold || (max_y - min_y) > spread_threshold;
}

int main()
{
    graph_type g;
    build_grid(g);

    auto index_map = boost::get(boost::vertex_index, g);
    std::vector< point > positions(boost::num_vertices(g));
    position_map position(positions.begin(), index_map);
    topology space;

    // unweighted layout uses BFS hop distances
    boost::gursoy_atun_layout(g, space, position);
    BOOST_TEST(layout_is_valid(positions));

    // weighted layout uses Dijkstra distances over the edge weights
    auto edge_weights = boost::get(boost::edge_weight, g);
    const int nsteps = static_cast< int >(boost::num_vertices(g));
    const double diameter_initial = std::sqrt(static_cast< double >(boost::num_vertices(g)));
    const double diameter_final = 1.0;
    const double learning_initial = 0.8;
    const double learning_final = 0.2;
    boost::gursoy_atun_layout(g, space, position, nsteps, diameter_initial, diameter_final, learning_initial, learning_final, index_map, edge_weights);
    BOOST_TEST(layout_is_valid(positions));

    return boost::report_errors();
}
