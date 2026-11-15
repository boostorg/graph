//=======================================================================
// Copyright 2026 Matyas W Egyhazy
// Copyright (C) 2026 Arnaud Becheler
// Author: Matyas W Egyhazy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP
#define BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP

#include <boost/assert.hpp>
#include <boost/concept/assert.hpp>
#include <boost/concept_check.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/simple_point.hpp>
#include <boost/static_assert.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>

namespace boost
{
namespace graph
{
    /**
     * Calculates the 2D Euclidean distance between two points using
     * std::hypot.
     *
     * Works with any point type possessing public '.x' and '.y' members.
     * Use your own distance functor (e.g. boost::geometry distance() 
     * if your point type does not have those members or don't want euclidean distances.
     */
    struct default_euclidean_distance
    {
        template < typename Point1, typename Point2 >
        auto constexpr operator()(Point1 const&  p1, Point2 const&  p2) const
        {
            return std::hypot(p1.x - p2.x, p1.y - p2.y);
        }
    };

// Connects every pair of vertices with an edge weighted by the distance between
// the corresponding points. A common preprocessing step for TSP algorithms.
//
// The distance functor must be supplied by the caller.
//
// Preconditions: num_vertices(g) == points.size() and g has no edges.
// Complexity: O(V^2).
template < typename VertexListGraph, typename PointContainer, typename WeightMap, typename VertexIndexMap, typename BinaryFunction >
    void connect_all_geometric(VertexListGraph& g, PointContainer const& points,
        WeightMap wmap, VertexIndexMap vmap, BinaryFunction distance)
    {
    using Traits = ::boost::graph_traits< VertexListGraph >;

    BOOST_CONCEPT_ASSERT((::boost::VertexListGraphConcept< VertexListGraph >));
    BOOST_CONCEPT_ASSERT((::boost::MutableGraphConcept< VertexListGraph >));
    BOOST_CONCEPT_ASSERT((::boost::RandomAccessContainerConcept< PointContainer >));
    BOOST_CONCEPT_ASSERT((::boost::ReadablePropertyMapConcept< VertexIndexMap, typename Traits::vertex_descriptor >));
    BOOST_CONCEPT_ASSERT((::boost::WritablePropertyMapConcept< WeightMap, typename Traits::edge_descriptor >));

    BOOST_STATIC_ASSERT_MSG((!std::is_convertible< typename Traits::directed_category, ::boost::directed_tag >::value), "connect_all_geometric requires an undirected graph type.");

    using WeightType = typename ::boost::property_traits< WeightMap >::value_type;

    BOOST_STATIC_ASSERT_MSG(!std::is_integral< WeightType >::value, "connect_all_geometric requires a non-integral weight type. Integer weights truncate distances.");

    BOOST_ASSERT_MSG(num_vertices(g) == points.size(), "connect_all_geometric requires num_vertices(g) == points.size()");

    auto verts(vertices(g));

    for (auto src(verts.first); src != verts.second; ++src)
    {
        auto const src_index = get(vmap, *src);
        auto dest(src);
        ++dest;

        for (; dest != verts.second; ++dest)
        {
            auto const dest_index = get(vmap, *dest);
            auto const weight = static_cast< WeightType >(
                distance(points[src_index], points[dest_index]));
            put(wmap, add_edge(*src, *dest, g).first, weight);
        }
    }
}

// Writes num_points distinct points to out, in generation order.
//
// Uniqueness is decided on the drawn coordinate pair rather than on PointType, so
// PointType needs no hash and no equality of its own. That is equivalent as long
// as PointType{x, y} is injective. A point type that normalises or snaps on
// construction needs its own filter.
//
// Returns the number of points actually written, which is less than num_points
// when the distributions cannot supply enough distinct values within
// max_attempts draws. A max_attempts of zero selects a default budget.
template < typename PointType, typename OutputIterator, typename XDistribution, typename YDistribution, typename URBG >
std::size_t generate_unique_random_points(std::size_t num_points, XDistribution x_dist, YDistribution y_dist, OutputIterator out, URBG&& gen, std::size_t max_attempts = 0)
{
    using CoordType = typename XDistribution::result_type;

    BOOST_STATIC_ASSERT_MSG((std::is_same< CoordType, typename YDistribution::result_type >::value), "X and Y distributions must have the same result type");

    if (max_attempts == 0)
        max_attempts = std::max< std::size_t >(10 * num_points, 100);

    ::boost::unordered_flat_set< std::pair< CoordType, CoordType > > seen;
    seen.reserve(num_points);

    std::size_t attempts = 0;

    while (seen.size() < num_points && attempts < max_attempts)
    {
        auto const x = x_dist(gen);
        auto const y = y_dist(gen);

        if (seen.insert(std::make_pair(x, y)).second)
            *out++ = PointType { x, y };

        ++attempts;
    }

    return seen.size();
}

// Populates g with num_points random points and complete geometric weights with parameterized X and Y distributions.
// Useful for testing algorithms on random complete geometric graphs (e.g. TSP algorithms).  
// Note:  Could be generalized to work on 3D+ spaces using variadic templates for the point type and
// distributions
//
// Throws std::runtime_error when the distributions cannot supply num_points
// distinct points, because a shorter point set would leave connect_all_geometric
// indexing past the end.
template < typename PointType, typename VertexListGraph, typename WeightMap, typename VertexIndexMap, typename XDistribution, typename YDistribution, typename URBG, typename BinaryFunction >
void generate_random_complete_geometric_graph(VertexListGraph& g, std::size_t num_points, XDistribution x_dist, YDistribution y_dist, WeightMap weight_map, VertexIndexMap vertex_index_map, URBG&& gen, BinaryFunction distance)
{
    std::vector< PointType > points;
    points.reserve(num_points);

    std::size_t const generated = generate_unique_random_points< PointType >(num_points, x_dist, y_dist, std::back_inserter(points), gen);

    if (generated != num_points)
        throw std::runtime_error("generate_random_complete_geometric_graph: the given distributions cannot supply num_points distinct points");

    connect_all_geometric(g, points, weight_map, vertex_index_map, distance);
}

} // end namespace graph
} // end namespace boost

#endif // BOOST_GRAPH_GEOMETRIC_GRAPH_GENERATOR_HPP
