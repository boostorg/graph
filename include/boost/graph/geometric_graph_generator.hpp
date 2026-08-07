//=======================================================================
// Copyright 2026
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP
#define BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/static_assert.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/geometry.hpp>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>
#include <boost/geometry/geometries/point_xy.hpp>


namespace boost
{

    // Traits class for coordinate access abstraction
template < typename Point, typename Enable = void >
struct geometric_point_traits
{
    static auto x(const Point& p) -> decltype(p.x) { return p.x; }
    static auto y(const Point& p) -> decltype(p.y) { return p.y; }
};

/// Partial specialization for all Boost.Geometry point types
template < typename Point >
struct geometric_point_traits< Point,
    typename std::enable_if<
        std::is_same< typename boost::geometry::traits::tag< Point >::type,
            boost::geometry::point_tag >::value >::type >
{
    static auto x(const Point& p) -> decltype(boost::geometry::get< 0 >(p))
    {
        return boost::geometry::get< 0 >(p);
    }
    static auto y(const Point& p) -> decltype(boost::geometry::get< 1 >(p))
    {
        return boost::geometry::get< 1 >(p);
    }
};

// connect_all_geometric
//
// Creates a complete graph with geometric distance edge weights.
// Connects all vertices in the graph with edges weighted by the
// distance between their corresponding points in the point container.
// This is a common preprocessing step for TSP algorithms.  
// Relies on on a templated distance function (e.g. boost::geometry::distance)
// for flexibility and compatibility with various point types, including
// Boost.Geometry points.
//
// Preconditions: g must have num_vertices(g) == points.size() and no edges
// Postconditions: g will be a complete graph with geometric distance weights
// Complexity: O(V^2) where V is the number of vertices

template < typename VertexListGraph, typename PointContainer,
    typename WeightMap, typename VertexIndexMap >
void connect_all_geometric(VertexListGraph& g, const PointContainer& points,
    WeightMap wmap, VertexIndexMap vmap)
{
    using boost::geometry::distance; 
    BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept< VertexIndexMap,
        typename graph_traits< VertexListGraph >::vertex_descriptor >));
    BOOST_CONCEPT_ASSERT((RandomAccessContainerConcept< PointContainer >));
    BOOST_CONCEPT_ASSERT((VertexListGraphConcept< VertexListGraph >));
    BOOST_CONCEPT_ASSERT((MutableGraphConcept< VertexListGraph >));
    BOOST_CONCEPT_ASSERT((WritablePropertyMapConcept< WeightMap,
        typename graph_traits< VertexListGraph >::edge_descriptor >));
    BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept< VertexIndexMap,
        typename graph_traits< VertexListGraph >::vertex_descriptor >));

    // Precondition: Graph should have no edges and size of points should match
    // num_vertices(g)
    BOOST_ASSERT_MSG(boost::num_edges(g) == 0,
        "connect_all_geometric requires a graph with no edges)");
    BOOST_ASSERT_MSG(boost::num_vertices(g) == points.size(),
        "connect_all_geometric requires num_vertices(g) == points.size()");

    using Edge = typename graph_traits< VertexListGraph >::edge_descriptor;
    using VItr = typename graph_traits< VertexListGraph >::vertex_iterator;

    // Deduce the weight type from the WeightMap's value type
    using WeightType = typename boost::property_traits< WeightMap >::value_type;

    using IndexType = typename boost::property_traits<VertexIndexMap>::value_type;

    // Compile-time assertion: Prevent integer weight types
    BOOST_STATIC_ASSERT_MSG(
        std::is_floating_point<WeightType>::value,
        "connect_all_geometric requires floating-point weight types (float, double, or long double). "
        "Integer types cause truncation and produce non-useful edge lengths. "
        "e.g. Use property<edge_weight_t, double> instead of property<edge_weight_t, int>."
    );

    std::pair< VItr, VItr > verts(vertices(g));

    for (VItr src(verts.first); src != verts.second; ++src)
    {
        const IndexType src_idx = boost::get(vmap, *src);  // Cache source index lookup

        VItr dest(src);
        ++dest;  // Skip self-edge

        for (; dest != verts.second; ++dest)
        {
            const IndexType dest_idx
                = boost::get(vmap, *dest); // Cache destination index lookup

            // Use templated distance function for compatibility with
            // Boost.Geometry.  In the case of boost::simple_point, this uses euclidean
            // (std::hypot)
            const WeightType weight = static_cast< WeightType >(
                distance(points[src_idx], points[dest_idx]));

            // No need to check 'inserted' - building fresh complete graph
            Edge e = boost::add_edge(*src, *dest, g).first;
            boost::put(wmap, e, weight);
        }
    }
}

// generate_random_points
//
// Generates a set of random unique 2D points .
// Uses unordered_set to ensure uniqueness and avoid duplicate points.
// This involves copying points into the output iterator...
// but simplifies uniqueness handling.
// Supports custom random distributions for flexible point generation patterns.
//
// Returns: The number of unique points generated.
//
// Parameters:
//   num_points - Number of unique points to generate
//   x_dist - Distribution for x-coordinates (e.g., uniform, normal)
//   y_dist - Distribution for y-coordinates (can differ from x)
//   out - Output iterator for storing generated points
//   rng - Random number generator (default: std::mt19937 with random seed)
//
// Postconditions: Exactly num_points unique points written to out
// Complexity: O(N) average case, O(N^2) worst case due to collision handling

// Generic version: PointType must be constructible from (CoordType, CoordType)
template < typename PointType, typename OutputIterator, typename XDistribution,
    typename YDistribution, typename RandomEngine >
std::size_t generate_random_points(
    std::size_t num_points,
    XDistribution x_dist,
    YDistribution y_dist,
    OutputIterator out,
    RandomEngine& rng,
    std::size_t max_attempts = 0)
{
    using CoordType = typename XDistribution::result_type;
    BOOST_STATIC_ASSERT_MSG(
        (std::is_same< CoordType, typename YDistribution::result_type >::value),
        "X and Y distributions must have the same result type");

    // This avoids the rare case in which the while loop runs indefinitely due to collisions 
    // when num_points is large and the distribution is narrow.
    if (max_attempts == 0)
        max_attempts = std::max<std::size_t>(10 * num_points, 100);


    // Hash and equality for generic PointType using traits
    auto generic_point_hash = [](const PointType& p) {
        std::size_t seed = 0;
        boost::hash_combine(seed, geometric_point_traits<PointType>::x(p));
        boost::hash_combine(seed, geometric_point_traits<PointType>::y(p));
        return seed;
    };
    auto generic_point_equal = [](const PointType& lhs, const PointType& rhs) {
        return geometric_point_traits<PointType>::x(lhs) == geometric_point_traits<PointType>::x(rhs) &&
               geometric_point_traits<PointType>::y(lhs) == geometric_point_traits<PointType>::y(rhs);
    };

    using PointSet = boost::unordered_flat_set< PointType, decltype(generic_point_hash), decltype(generic_point_equal) >;
    PointSet point_set(0, generic_point_hash, generic_point_equal);
    point_set.reserve(num_points);

    std::size_t attempts = 0;
    while (point_set.size() < num_points && attempts < max_attempts)
    {
        point_set.insert(PointType{ x_dist(rng), y_dist(rng) });
        ++attempts;
    }

    std::copy(std::make_move_iterator(point_set.begin()),
        std::make_move_iterator(point_set.end()), out);

    return point_set.size();
}

// Overload with default RNG and max_attempts parameter
template < typename PointType, typename OutputIterator, typename XDistribution,
    typename YDistribution >
std::size_t generate_random_points(
    std::size_t num_points,
    XDistribution x_dist,
    YDistribution y_dist,
    OutputIterator out,
    std::size_t max_attempts = 0)
{
    std::mt19937 rng(std::random_device {}());
    return generate_random_points<PointType>(num_points, x_dist, y_dist, out, rng, max_attempts);
}

// Overload for uniform distribution, with max_attempts parameter
template < typename PointType, typename OutputIterator, typename CoordType = double >
std::size_t generate_random_points(
    std::size_t num_points,
    std::size_t coordinate_max,
    OutputIterator out,
    std::size_t max_attempts = 0)
{
    std::uniform_real_distribution< CoordType > dist(
        static_cast< CoordType >(0), static_cast< CoordType >(coordinate_max));
    return generate_random_points<PointType>(num_points, dist, dist, out, max_attempts);
}

// make_random_euclidean_graph
//
// Creates a complete graph with random points and Euclidean distance weights.
// This is a convenience function that combines random point generation with
// complete graph construction for TSP testing and benchmarking.
//
// Parameters:
//   g - Graph to populate (must have num_vertices(g) == num_points)
//   num_points - Number of vertices/points
//   coordinate_max - Maximum coordinate value for random points
//   weight_map - Property map for storing edge weights
//   vertex_index_map - Property map for vertex indices
//
// Postconditions: g is a complete graph with Euclidean distance weights
// Complexity: O(V^2) where V is the number of vertices
template < typename VertexListGraph, typename WeightMap,
    typename VertexIndexMap, typename CoordType = double >
void make_random_euclidean_graph(VertexListGraph& g, std::size_t num_points,
    std::size_t coordinate_max, WeightMap weight_map,
    VertexIndexMap vertex_index_map)
{
    std::vector< simple_point< CoordType > > points;
    points.reserve(num_points);
    generate_random_points< simple_point< CoordType > >(
        num_points, coordinate_max, std::back_inserter(points));
    connect_all_geometric(g, points, weight_map, vertex_index_map);
}

// make_random_euclidean_graph (parameterized distribution version)
//
// Version with custom distribution support for flexible point generation.
template < typename VertexListGraph, typename WeightMap,
    typename VertexIndexMap, typename XDistribution, typename YDistribution >
void make_random_euclidean_graph(VertexListGraph& g, std::size_t num_points,
    XDistribution x_dist, YDistribution y_dist, WeightMap weight_map,
    VertexIndexMap vertex_index_map)
{
    using CoordType = typename XDistribution::result_type;
    std::vector< simple_point< CoordType > > points;
    points.reserve(num_points);
    generate_random_points< simple_point< CoordType > >(
        num_points, x_dist, y_dist, std::back_inserter(points));
    connect_all_geometric(g, points, weight_map, vertex_index_map);
}


// make_random_geometric_graph (parameterized distribution version)
//
// Creates a complete graph with random points of arbitrary PointType and geometric distance weights.
// This function allows the user to specify the point type and random distributions for each coordinate.
//
// Parameters:
//   g - Graph to populate (must have num_vertices(g) == num_points)
//   num_points - Number of vertices/points
//   x_dist, y_dist - Distributions for x and y coordinates
//   weight_map - Property map for storing edge weights
//   vertex_index_map - Property map for vertex indices
//
// Postconditions: g is a complete graph with geometric distance weights
// Complexity: O(V^2) where V is the number of vertices
template < typename PointType, typename VertexListGraph, typename WeightMap,
    typename VertexIndexMap, typename XDistribution, typename YDistribution >
void make_random_geometric_graph(VertexListGraph& g, std::size_t num_points,
    XDistribution x_dist, YDistribution y_dist, WeightMap weight_map,
    VertexIndexMap vertex_index_map)
{
    std::vector< PointType > points;
    points.reserve(num_points);
    generate_random_points<PointType>(num_points, x_dist, y_dist, std::back_inserter(points));
    connect_all_geometric(g, points, weight_map, vertex_index_map);
}


// make_random_geometric_graph
//
// Creates a complete graph with random points of arbitrary PointType and geometric distance weights.
// This overload uses a uniform real distribution for both coordinates in the range [0, coordinate_max].
//
// Parameters:
//   g - Graph to populate (must have num_vertices(g) == num_points)
//   num_points - Number of vertices/points
//   coordinate_max - Maximum coordinate value for random points
//   weight_map - Property map for storing edge weights
//   vertex_index_map - Property map for vertex indices
//
// Postconditions: g is a complete graph with geometric distance weights
// Complexity: O(V^2) where V is the number of vertices
template < typename PointType, typename VertexListGraph, typename WeightMap,
    typename VertexIndexMap, typename CoordType = double >
void make_random_geometric_graph(VertexListGraph& g, std::size_t num_points,
    std::size_t coordinate_max, WeightMap weight_map,
    VertexIndexMap vertex_index_map)
{
    std::vector< PointType > points;
    points.reserve(num_points);
    generate_random_points<PointType>(num_points, coordinate_max, std::back_inserter(points));
    connect_all_geometric(g, points, weight_map, vertex_index_map);
}

} // namespace boost

#endif // BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP
