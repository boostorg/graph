//=======================================================================
// Copyright 2026
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#define BOOST_TEST_MODULE geometric_graph_generator_test
#include <boost/test/included/unit_test.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/geometric_graph_generator.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/geometry.hpp>
#include <boost/geometry/geometries/point.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

namespace
{

// Type aliases for common graph types used in tests
using UndirectedListGraph
    = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, double > >;

using UndirectedMatrixGraph = boost::adjacency_matrix< boost::undirectedS,
    boost::no_property, boost::property< boost::edge_weight_t, double > >;

using PointDbl = boost::simple_point< double >;

// Helper: Calculate Euclidean distance between two points
template < typename CoordType >
double euclidean_distance(const boost::simple_point< CoordType >& p1,
    const boost::simple_point< CoordType >& p2)
{
    return std::hypot(p1.x - p2.x, p1.y - p2.y);
}

// Helper: Verify that a graph is complete
template < typename Graph >
bool is_complete_graph(const Graph& g)
{
    const std::size_t n = boost::num_vertices(g);
    const std::size_t expected_edges = (n * (n - 1)) / 2;
    return boost::num_edges(g) == expected_edges;
}

//=======================================================================
// Test Fixtures
//=======================================================================

// Fixture for tests using random points with matrix graph
template < typename Graph >
struct RandomGraphFixture
{
    static constexpr std::size_t num_vertices = 10;
    Graph g;
    std::vector< PointDbl > points;

    RandomGraphFixture() : g(num_vertices)
    {
        boost::generate_random_points< PointDbl >(
            num_vertices, 100, std::back_inserter(points));
        boost::connect_all_geometric(g, points,
            boost::get(boost::edge_weight, g),
            boost::get(boost::vertex_index, g));
    }
};

using MatrixGraphFixture = RandomGraphFixture< UndirectedMatrixGraph >;
using ListGraphFixture = RandomGraphFixture< UndirectedListGraph >;

// Fixture for tests using known 3-4-5 triangle points
template < typename Graph, typename PointType >
struct KnownPointsFixture
{
    Graph g;
    std::vector< PointType > points;
    decltype(boost::get(boost::edge_weight, g)) weight_map;

    KnownPointsFixture() : g(5), weight_map(boost::get(boost::edge_weight, g))
    {
        points = { { 0.0, 0.0 }, { 3.0, 0.0 }, { 0.0, 4.0 }, { 3.0, 4.0 },
            { 1.5, 2.0 } };
        boost::connect_all_geometric(
            g, points, weight_map, boost::get(boost::vertex_index, g));
    }
};

using MatrixKnownPointsFixture
    = KnownPointsFixture< UndirectedMatrixGraph, PointDbl >;

} // anonymous namespace

//=======================================================================
// Test Suite: generate_random_points
//=======================================================================

BOOST_AUTO_TEST_SUITE(generate_random_points_tests)

BOOST_AUTO_TEST_CASE(test_uniqueness)
{
    const std::size_t num_points = 100;
    std::vector< PointDbl > points;

    boost::generate_random_points< PointDbl >(
        num_points, 10000, std::back_inserter(points));

    using Pair = std::pair< double, double >;
    boost::unordered_flat_set< Pair, boost::hash< Pair > > unique_points;
    for (const auto& p : points)
    {
        auto result = unique_points.emplace(p.x, p.y);
        BOOST_TEST(result.second); // Assert immediately if duplicate found
    }
}

BOOST_AUTO_TEST_CASE(test_custom_distributions)
{
    const std::size_t num_points = 30;
    std::vector< PointDbl > points;

    std::uniform_real_distribution< double > x_dist(0.0, 100.0);
    std::uniform_real_distribution< double > y_dist(0.0, 200.0);

    std::size_t generated = boost::generate_random_points< PointDbl >(
        num_points, x_dist, y_dist, std::back_inserter(points));

    BOOST_TEST(generated == num_points);
    BOOST_TEST(points.size() == num_points);

    for (const auto& p : points)
    {
        BOOST_TEST(p.x >= 0.0);
        BOOST_TEST(p.x <= 100.0);
        BOOST_TEST(p.y >= 0.0);
        BOOST_TEST(p.y <= 200.0);
    }
}

BOOST_AUTO_TEST_CASE(test_max_attempts)
{
    const std::size_t num_points = 100;
    std::uniform_int_distribution< int > narrow_dist(0, 9);
    std::mt19937 rng(12345);
    std::vector< boost::simple_point< int > > points;

    std::size_t generated
        = boost::generate_random_points < boost::simple_point< int > >(num_points,
        narrow_dist, narrow_dist, std::back_inserter(points), rng, 200);

    BOOST_TEST(generated < num_points);
    BOOST_TEST(points.size() == generated);
}

BOOST_AUTO_TEST_CASE(test_empty)
{
    std::vector< PointDbl > points;
    std::size_t generated
        = boost::generate_random_points< PointDbl >(0, 100, std::back_inserter(points));

    BOOST_TEST(generated == 0u);
    BOOST_TEST(points.empty());
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_CASE(test_boost_geometry_point_compatibility)
{
    using BoostGeomPoint = boost::geometry::model::d2::point_xy<double>;
    using Graph = boost::adjacency_matrix< boost::undirectedS, boost::no_property, boost::property< boost::edge_weight_t, double > >;

    std::vector<BoostGeomPoint> points = {
        BoostGeomPoint(0.0, 0.0),
        BoostGeomPoint(3.0, 0.0),
        BoostGeomPoint(0.0, 4.0)
    };
    Graph g(points.size());
    auto weight_map = boost::get(boost::edge_weight, g);
    auto vertex_index_map = boost::get(boost::vertex_index, g);

    boost::connect_all_geometric(g, points, weight_map, vertex_index_map);

    // Check edge weights using Boost.Geometry distance
    auto e01 = boost::edge(0, 1, g);
    BOOST_REQUIRE(e01.second);
    double expected01 = boost::geometry::distance(points[0], points[1]);
    BOOST_TEST(boost::get(weight_map, e01.first) == expected01, boost::test_tools::tolerance(1e-10));

    auto e02 = boost::edge(0, 2, g);
    BOOST_REQUIRE(e02.second);
    double expected02 = boost::geometry::distance(points[0], points[2]);
    BOOST_TEST(boost::get(weight_map, e02.first) == expected02, boost::test_tools::tolerance(1e-10));

    auto e12 = boost::edge(1, 2, g);
    BOOST_REQUIRE(e12.second);
    double expected12 = boost::geometry::distance(points[1], points[2]);
    BOOST_TEST(boost::get(weight_map, e12.first) == expected12, boost::test_tools::tolerance(1e-10));
}

//=======================================================================
// Test Suite: connect_all_geometric (using fixtures)
//=======================================================================

BOOST_AUTO_TEST_SUITE(connect_all_euclidean_tests)

BOOST_FIXTURE_TEST_CASE(test_adjacency_matrix, MatrixGraphFixture)
{
    BOOST_TEST(is_complete_graph(g));
}

BOOST_FIXTURE_TEST_CASE(test_adjacency_list, ListGraphFixture)
{
    BOOST_TEST(is_complete_graph(g));
}

BOOST_FIXTURE_TEST_CASE(test_edge_weights_matrix, MatrixKnownPointsFixture)
{
    auto e01 = boost::edge(0, 1, g);
    BOOST_REQUIRE(e01.second);
    BOOST_TEST(boost::get(weight_map, e01.first)
            == euclidean_distance(points[0], points[1]),
        boost::test_tools::tolerance(1e-10));

    auto e02 = boost::edge(0, 2, g);
    BOOST_REQUIRE(e02.second);
    BOOST_TEST(boost::get(weight_map, e02.first)
            == euclidean_distance(points[0], points[2]),
        boost::test_tools::tolerance(1e-10));

    auto e03 = boost::edge(0, 3, g);
    BOOST_REQUIRE(e03.second);
    BOOST_TEST(boost::get(weight_map, e03.first)
            == euclidean_distance(points[0], points[3]),
        boost::test_tools::tolerance(1e-10));
}

BOOST_AUTO_TEST_CASE(test_single_vertex_matrix)
{
    UndirectedMatrixGraph g(1);
    std::vector< PointDbl > points = { { 0.0, 0.0 } };

    boost::connect_all_geometric(g, points, boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));

    BOOST_TEST(boost::num_edges(g) == 0u);
}

BOOST_AUTO_TEST_SUITE_END()

//=======================================================================
// Test Suite: make_random_euclidean_graph
//=======================================================================

BOOST_AUTO_TEST_SUITE(make_random_euclidean_graph_tests)

BOOST_AUTO_TEST_CASE(test_with_distributions_matrix)
{
    const std::size_t num_vertices = 20;
    UndirectedMatrixGraph g(num_vertices);

    std::uniform_real_distribution< double > x_dist(0.0, 500.0);
    std::normal_distribution< double > y_dist(250.0, 50.0);

    boost::make_random_euclidean_graph(g, num_vertices, x_dist, y_dist,
        boost::get(boost::edge_weight, g), boost::get(boost::vertex_index, g));

    BOOST_TEST(is_complete_graph(g));
}

BOOST_AUTO_TEST_SUITE_END()

//=======================================================================
// Test Suite: float_precision
//=======================================================================

BOOST_AUTO_TEST_SUITE(float_precision_tests)

BOOST_AUTO_TEST_CASE(test_float_generation_and_math)
{
    const std::size_t num_points = 20;
    std::vector< boost::simple_point< float > > points;
    std::uniform_real_distribution< float > dist(0.0f, 100.0f);

    std::size_t generated = boost::generate_random_points< boost::simple_point< float > >(
        num_points, dist, dist, std::back_inserter(points));

    BOOST_TEST(generated == num_points);

    using FloatGraph = boost::adjacency_matrix< boost::undirectedS,
        boost::no_property, boost::property< boost::edge_weight_t, float > >;

    FloatGraph g(3);
    std::vector< boost::simple_point< float > > known_points
        = { { 0.0f, 0.0f }, { 3.0f, 0.0f }, { 0.0f, 4.0f } };

    boost::connect_all_geometric(g, known_points,
        boost::get(boost::edge_weight, g), boost::get(boost::vertex_index, g));

    auto weight_map = boost::get(boost::edge_weight, g);

    auto e01 = boost::edge(0, 1, g);
    BOOST_REQUIRE(e01.second);
    BOOST_TEST(boost::get(weight_map, e01.first)
            == euclidean_distance(known_points[0], known_points[1]),
        boost::test_tools::tolerance(1e-5f));

    auto e02 = boost::edge(0, 2, g);
    BOOST_REQUIRE(e02.second);
    BOOST_TEST(boost::get(weight_map, e02.first)
            == euclidean_distance(known_points[0], known_points[2]),
        boost::test_tools::tolerance(1e-5f));

    auto e12 = boost::edge(1, 2, g);
    BOOST_REQUIRE(e12.second);
    BOOST_TEST(boost::get(weight_map, e12.first)
            == euclidean_distance(known_points[1], known_points[2]),
        boost::test_tools::tolerance(1e-5f));
}

BOOST_AUTO_TEST_SUITE_END()

//=======================================================================
// Test Suite: triangle_inequality
//=======================================================================

BOOST_AUTO_TEST_SUITE(triangle_inequality_tests)

BOOST_FIXTURE_TEST_CASE(
    test_euclidean_weights_satisfy_triangle_inequality_matrix,
    MatrixGraphFixture)
{
    auto weight_map = boost::get(boost::edge_weight, g);

    for (std::size_t i = 0; i < num_vertices; ++i)
    {
        for (std::size_t j = 0; j < num_vertices; ++j)
        {
            if (i == j)
                continue;
            for (std::size_t k = 0; k < num_vertices; ++k)
            {
                if (k == i || k == j)
                    continue;

                auto e_ik = boost::edge(i, k, g);
                auto e_ij = boost::edge(i, j, g);
                auto e_jk = boost::edge(j, k, g);

                double d_ik = boost::get(weight_map, e_ik.first);
                double d_ij = boost::get(weight_map, e_ij.first);
                double d_jk = boost::get(weight_map, e_jk.first);

                BOOST_TEST(d_ik <= d_ij + d_jk + 1e-10);
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()

//=======================================================================
// Test Suite: make_random_geometric_graph
//=======================================================================

BOOST_AUTO_TEST_SUITE(make_random_geometric_graph_tests)

BOOST_AUTO_TEST_CASE(test_with_uniform_distribution_simple_point)
{
    using Graph = UndirectedMatrixGraph;
    using Point = boost::simple_point<double>;
    const std::size_t num_vertices = 15;
    const std::size_t coordinate_max = 1000;
    Graph g(num_vertices);
    boost::make_random_geometric_graph< Point >(
        g, num_vertices, coordinate_max,
        boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));
    BOOST_TEST(is_complete_graph(g));
}

BOOST_AUTO_TEST_CASE(test_with_uniform_distribution_boost_geometry_point)
{
    using Graph = UndirectedMatrixGraph;
    using Point = boost::geometry::model::d2::point_xy<double>;
    const std::size_t num_vertices = 12;
    const std::size_t coordinate_max = 500;
    Graph g(num_vertices);
    boost::make_random_geometric_graph< Point >(
        g, num_vertices, coordinate_max,
        boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));
    BOOST_TEST(is_complete_graph(g));
}

BOOST_AUTO_TEST_CASE(test_with_custom_distribution_boost_geometry_point)
{
    using Graph = UndirectedMatrixGraph;
    using Point = boost::geometry::model::d2::point_xy<double>;
    const std::size_t num_vertices = 10;
    std::uniform_real_distribution<double> x_dist(0.0, 100.0);
    std::normal_distribution<double> y_dist(50.0, 10.0);
    Graph g(num_vertices);
    boost::make_random_geometric_graph< Point >(
        g, num_vertices, x_dist, y_dist,
        boost::get(boost::edge_weight, g),
        boost::get(boost::vertex_index, g));
    BOOST_TEST(is_complete_graph(g));
}

BOOST_AUTO_TEST_SUITE_END()