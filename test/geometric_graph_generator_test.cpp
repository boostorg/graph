//=======================================================================
// Copyright 2026 Matyas W Egyhazy
// Copyright (C) 2026 Arnaud Becheler
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/geometric_graph_generator.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>

#include <cmath>
#include <cstddef>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{

using Graph = ::boost::adjacency_matrix< ::boost::undirectedS, ::boost::no_property, ::boost::property< ::boost::edge_weight_t, double > >;
using Point = ::boost::simple_point< double >;

// ADL on simple_point resolves to its hidden friend. Safe outside namespace boost.
struct euclidean
{
    template < typename P > auto operator()(P const& a, P const& b) const -> decltype(distance(a, b))
    {
        return distance(a, b);
    }
};

// Stands in for a third party point type such as boost::geometry point_xy.
// No hash, no equality, no default constructor.
struct opaque_point
{
    double a;
    double b;

    opaque_point(double x, double y) : a(x), b(y) { }
};

struct opaque_distance
{
    double operator()(opaque_point const& p, opaque_point const& q) const
    {
        return std::hypot(p.a - q.a, p.b - q.b);
    }
};

// A 3-4-5 triangle gives exact weights with no tolerance needed.
void test_known_weights()
{
    Graph g(3);
    const std::vector< Point > points = { { 0.0, 0.0 }, { 3.0, 0.0 }, { 0.0, 4.0 } };
    auto weight_map = ::boost::get(::boost::edge_weight, g);

    ::boost::graph::connect_all_geometric(g, points, weight_map, ::boost::get(::boost::vertex_index, g), euclidean {});

    BOOST_TEST_EQ(::boost::num_edges(g), 3u);
    BOOST_TEST_EQ(::boost::get(weight_map, ::boost::edge(0, 1, g).first), 3.0);
    BOOST_TEST_EQ(::boost::get(weight_map, ::boost::edge(0, 2, g).first), 4.0);
    BOOST_TEST_EQ(::boost::get(weight_map, ::boost::edge(1, 2, g).first), 5.0);
}

void test_single_vertex_has_no_edges()
{
    Graph g(1);
    const std::vector< Point > points = { { 0.0, 0.0 } };

    ::boost::graph::connect_all_geometric(g, points, ::boost::get(::boost::edge_weight, g), ::boost::get(::boost::vertex_index, g), euclidean {});

    BOOST_TEST_EQ(::boost::num_edges(g), 0u);
}

// The same seed must give the same weights, and points come out in generation order.
void test_reproducible()
{
    constexpr std::size_t num_points = 12;
    std::uniform_real_distribution< double > dist(0.0, 100.0);
    std::vector< double > weights[2];

    for (int run = 0; run < 2; ++run)
    {
        Graph g(num_points);
        std::mt19937 gen(42);
        auto weight_map = ::boost::get(::boost::edge_weight, g);

        ::boost::graph::make_random_geometric_graph< Point >(g, num_points, dist, dist, weight_map, ::boost::get(::boost::vertex_index, g), gen, euclidean {});

        BOOST_TEST_EQ(::boost::num_edges(g), num_points * (num_points - 1) / 2);

        const auto edge_range = ::boost::edges(g);

        for (auto it = edge_range.first; it != edge_range.second; ++it)
            weights[run].push_back(::boost::get(weight_map, *it));
    }

    BOOST_TEST(weights[0] == weights[1]);
}

void test_requested_count_is_met()
{
    constexpr std::size_t num_points = 20;
    std::uniform_real_distribution< double > dist(0.0, 1000.0);
    std::mt19937 gen(1);
    std::vector< Point > points;

    const std::size_t generated = ::boost::graph::generate_unique_random_points< Point >(num_points, dist, dist, std::back_inserter(points), gen);

    BOOST_TEST_EQ(generated, num_points);
    BOOST_TEST_EQ(points.size(), num_points);
}

// Nine lattice points cannot fill fifty vertices, so the shortfall must be reported
// rather than left for connect_all_geometric to index past the end.
void test_shortfall_throws()
{
    using IntPoint = ::boost::simple_point< int >;
    Graph g(50);
    std::uniform_int_distribution< int > narrow(0, 2);
    std::mt19937 gen(7);

    BOOST_TEST_THROWS((::boost::graph::make_random_geometric_graph< IntPoint >(g, 50, narrow, narrow, ::boost::get(::boost::edge_weight, g), ::boost::get(::boost::vertex_index, g), gen, euclidean {})), std::runtime_error);
}

void test_shortfall_is_reported_by_return_value()
{
    using IntPoint = ::boost::simple_point< int >;
    std::uniform_int_distribution< int > narrow(0, 2);
    std::mt19937 gen(7);
    std::vector< IntPoint > points;

    const std::size_t generated = ::boost::graph::generate_unique_random_points< IntPoint >(50, narrow, narrow, std::back_inserter(points), gen, 200);

    BOOST_TEST_LT(generated, 50u);
    BOOST_TEST_EQ(points.size(), generated);
}

// The generator must not require anything of PointType beyond construction from
// two coordinates, otherwise third party point types stop working.
void test_point_type_needs_no_hash_or_equality()
{
    constexpr std::size_t num_points = 10;
    Graph g(num_points);
    std::mt19937 gen(42);
    std::uniform_real_distribution< double > dist(0.0, 100.0);

    ::boost::graph::make_random_geometric_graph< opaque_point >(g, num_points, dist, dist, ::boost::get(::boost::edge_weight, g), ::boost::get(::boost::vertex_index, g), gen, opaque_distance {});

    BOOST_TEST_EQ(::boost::num_edges(g), num_points * (num_points - 1) / 2);
}

} // end anonymous namespace

int main()
{
    test_known_weights();
    test_single_vertex_has_no_edges();
    test_reproducible();
    test_requested_count_is_met();
    test_shortfall_throws();
    test_shortfall_is_reported_by_return_value();
    test_point_type_needs_no_hash_or_equality();
    return ::boost::report_errors();
}
