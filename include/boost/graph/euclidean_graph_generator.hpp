//=======================================================================
// Copyright 2026
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_EUCLIDEAN_GRAPH_GENERATOR_HPP
#define BOOST_GRAPH_EUCLIDEAN_GRAPH_GENERATOR_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/static_assert.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/unordered/unordered_flat_set.hpp>
#include <boost/concept/assert.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <algorithm>
#include <cmath>
#include <iterator>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace boost
{

// connect_all_euclidean
//
// Creates a complete graph with Euclidean distance edge weights.
// Connects all vertices in the graph with edges weighted by the Euclidean
// distance between their corresponding points in the point container.
// This is a common preprocessing step for TSP algorithms.
//
// Preconditions: g must have num_vertices(g) == points.size()
// Postconditions: g will be a complete graph with Euclidean distance weights
// Complexity: O(V^2) where V is the number of vertices

template < typename VertexListGraph, typename PointContainer,
    typename WeightMap, typename VertexIndexMap >
void connect_all_euclidean(VertexListGraph& g, const PointContainer& points,
    WeightMap wmap, VertexIndexMap vmap)
{
    BOOST_CONCEPT_ASSERT((VertexListGraphConcept< VertexListGraph >));
    BOOST_CONCEPT_ASSERT((WritablePropertyMapConcept< WeightMap,
        typename graph_traits< VertexListGraph >::edge_descriptor >));
    BOOST_CONCEPT_ASSERT((ReadablePropertyMapConcept< VertexIndexMap,
        typename graph_traits< VertexListGraph >::vertex_descriptor >));

    using Edge = typename graph_traits< VertexListGraph >::edge_descriptor;
    using VItr = typename graph_traits< VertexListGraph >::vertex_iterator;

    // Deduce the weight type from the WeightMap's value type
    using WeightType = typename boost::property_traits< WeightMap >::value_type;

    // Deduce coordinate type from the point container
    using PointType = typename PointContainer::value_type;
    using CoordType = decltype(std::declval<PointType>().x);
    using IndexType = typename boost::property_traits<VertexIndexMap>::value_type;


    // Compile-time assertion: Prevent integer weight types
    BOOST_STATIC_ASSERT_MSG(
        !std::is_integral<WeightType>::value,
        "connect_all_euclidean requires floating-point weight types (float, double, or long double). "
        "Integer types cause truncation and produce non-useful incorrect tour lengths. "
        "e.g. Use property<edge_weight_t, double> instead of property<edge_weight_t, int>."
    );


    std::pair< VItr, VItr > verts(vertices(g));

    for (VItr src(verts.first); src != verts.second; ++src)
    {
        const IndexType src_idx = get(vmap, *src);  // Cache source index lookup

        VItr dest(src);
        ++dest;  // Skip self-edge

        for (; dest != verts.second; ++dest)
        {
            const IndexType dest_idx = get(vmap, *dest);  // Cache destination index lookup

            // Promote to WeightType for calculation precision and to avoid overflow
            const WeightType dx =
                static_cast<WeightType>(points[src_idx].x) -
                static_cast<WeightType>(points[dest_idx].x);
            const WeightType dy =
                static_cast<WeightType>(points[src_idx].y) -
                static_cast<WeightType>(points[dest_idx].y);

            // Use std::hypot for numerically stable Euclidean distance
            const WeightType weight = static_cast<WeightType>(std::hypot(dx, dy));

            // No need to check 'inserted' - building fresh complete graph
            Edge e = add_edge(*src, *dest, g).first;
            put(wmap, e, weight);
        }
    }
}

// generate_random_points
//
// Generates a set of random unique 2D points for TSP testing.
// Uses std::unordered_set to ensure uniqueness and avoid duplicate points.
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
template < typename OutputIterator, typename XDistribution,
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

    auto point_hash = [](const simple_point< CoordType >& p)
    {
        std::size_t seed = 0;
        boost::hash_combine(seed, p.x);
        boost::hash_combine(seed, p.y);
        return seed;
    };

    auto point_equal = [](const simple_point< CoordType >& lhs,
                           const simple_point< CoordType >& rhs)
    { return lhs.x == rhs.x && lhs.y == rhs.y; };

    using Point = boost::simple_point< CoordType >;
    using PointSet = boost::unordered_flat_set< Point, decltype(point_hash),
        decltype(point_equal) >;

    PointSet point_set(0, point_hash, point_equal);
    point_set.reserve(num_points);

    std::size_t attempts = 0;
    while (point_set.size() < num_points && attempts < max_attempts)
    {
        point_set.insert(Point { x_dist(rng), y_dist(rng) });
        ++attempts;
    }

    std::copy(std::make_move_iterator(point_set.begin()),
        std::make_move_iterator(point_set.end()), out);

    return point_set.size();
}

// Overload with default RNG and max_attempts parameter
template < typename OutputIterator, typename XDistribution,
    typename YDistribution >
std::size_t generate_random_points(
    std::size_t num_points,
    XDistribution x_dist,
    YDistribution y_dist,
    OutputIterator out,
    std::size_t max_attempts = 0)
{
    std::mt19937 rng(std::random_device {}());
    return generate_random_points(num_points, x_dist, y_dist, out, rng, max_attempts);
}

// Overload for uniform distribution, with max_attempts parameter
template < typename OutputIterator, typename CoordType = double >
std::size_t generate_random_points(
    std::size_t num_points,
    std::size_t coordinate_max,
    OutputIterator out,
    std::size_t max_attempts = 0)
{
    std::uniform_real_distribution< CoordType > dist(
        static_cast< CoordType >(0), static_cast< CoordType >(coordinate_max));
    return generate_random_points(num_points, dist, dist, out, max_attempts);
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
    generate_random_points< decltype(std::back_inserter(points)), CoordType >(
        num_points, coordinate_max, std::back_inserter(points));
    connect_all_euclidean(g, points, weight_map, vertex_index_map);
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
    generate_random_points(
        num_points, x_dist, y_dist, std::back_inserter(points));
    connect_all_euclidean(g, points, weight_map, vertex_index_map);
}

} // namespace boost

#endif // BOOST_GRAPH_EUCLIDEAN_GRAPH_GENERATOR_HPP
