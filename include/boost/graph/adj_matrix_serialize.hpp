//=======================================================================
// Copyright 2026 Varun Sharma
// Authors: Varun Sharma
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_ADJ_MATRIX_SERIALIZE_HPP
#define BOOST_GRAPH_ADJ_MATRIX_SERIALIZE_HPP

#include <utility>

#include <boost/core/serialization.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/pending/property_serialize.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace boost
{

namespace serialization
{

    // Disable tracking for adjacency_matrix, which is not polymorphic. This
    // serializer writes graph structure and properties by value and rebuilds
    // edges on load; it does not preserve graph-object identity. Use this only
    // when pointer identity for graph objects need not be preserved.
    template < class T >
    struct tracking_level_impl;

    template < class Directed, class VP, class EP, class GP, class Allocator >
    struct tracking_level_impl<
        const boost::adjacency_matrix< Directed, VP, EP, GP, Allocator > >
    : boost::integral_constant< int, 0 >
    {
    };

    template < class Archive, class Directed, class VP, class EP, class GP, class Allocator >
    inline void save(Archive& ar,
        const boost::adjacency_matrix< Directed, VP, EP, GP, Allocator >& graph,
        const unsigned int /* version */)
    {
        const auto V = boost::num_vertices(graph);
        const auto E = boost::num_edges(graph);

        ar << BOOST_NVP(V);
        ar << BOOST_NVP(E);

        auto vertex_range = boost::vertices(graph);
        for (auto it = vertex_range.first; it != vertex_range.second; ++it)
        {
            auto&& vertex_property =
                boost::get(boost::vertex_all_t(), graph, *it);
            ar << BOOST_NVP(vertex_property);
        }

        auto edge_range = boost::edges(graph);
        for (auto it = edge_range.first; it != edge_range.second; ++it)
        {
            const auto u = boost::source(*it, graph);
            const auto v = boost::target(*it, graph);
            auto&& edge_property =
                boost::get(boost::edge_all_t(), graph, *it);

            ar << BOOST_NVP(u);
            ar << BOOST_NVP(v);
            ar << BOOST_NVP(edge_property);
        }

        ar << boost::make_nvp(
            "graph_property", boost::get_property(graph, boost::graph_all_t()));
    }

    template < class Archive, class Directed, class VP, class EP, class GP, class Allocator >
    inline void load(Archive& ar,
        boost::adjacency_matrix< Directed, VP, EP, GP, Allocator >& graph,
        const unsigned int /* version */)
    {
        using Graph = boost::adjacency_matrix< Directed, VP, EP, GP, Allocator >;
        using VertexSize = typename Graph::vertices_size_type;
        using EdgeSize = typename Graph::edges_size_type;
        using Vertex = typename Graph::vertex_descriptor;

        VertexSize V = 0;
        EdgeSize E = 0;

        ar >> BOOST_NVP(V);
        ar >> BOOST_NVP(E);

        Graph loaded(V);

        auto vertex_range = boost::vertices(loaded);
        for (auto it = vertex_range.first; it != vertex_range.second; ++it)
        {
            auto&& vertex_property =
                boost::get(boost::vertex_all_t(), loaded, *it);
            ar >> BOOST_NVP(vertex_property);
        }

        for (EdgeSize i = 0; i < E; ++i)
        {
            Vertex u = 0;
            Vertex v = 0;

            ar >> BOOST_NVP(u);
            ar >> BOOST_NVP(v);

            const auto e = boost::add_edge(u, v, loaded).first;
            auto&& edge_property = boost::get(boost::edge_all_t(), loaded, e);
            ar >> BOOST_NVP(edge_property);
        }

        ar >> boost::make_nvp("graph_property",
            boost::get_property(loaded, boost::graph_all_t()));

        graph = std::move(loaded);
    }

    template < class Archive, class Directed, class VP, class EP, class GP, class Allocator >
    inline void serialize(Archive& ar,
        boost::adjacency_matrix< Directed, VP, EP, GP, Allocator >& graph,
        const unsigned int version)
    {
        boost::core::split_free(ar, graph, version);
    }

} // namespace serialization
} // namespace boost

#endif // BOOST_GRAPH_ADJ_MATRIX_SERIALIZE_HPP
