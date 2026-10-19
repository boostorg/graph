//=======================================================================
// Copyright 2026 Varun Sharma
// Authors: Varun Sharma
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_CSR_GRAPH_SERIALIZE_HPP
#define BOOST_GRAPH_CSR_GRAPH_SERIALIZE_HPP

#include <cstddef>
#include <utility>
#include <vector>

#include <boost/core/serialization.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/pending/property_serialize.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace boost
{

namespace serialization
{

    // This serializer writes graph structure and properties by value and
    // rebuilds edges on load; it does not preserve graph-object identity.
    template < class T >
    struct tracking_level_impl;

    template < class Directed, class VP, class EP, class GP, class Vertex,
        class EdgeIndex >
    struct tracking_level_impl< const boost::compressed_sparse_row_graph<
        Directed, VP, EP, GP, Vertex, EdgeIndex > >
    : boost::integral_constant< int, 0 >
    {
    };

    template < class Archive, class Directed, class VP, class EP, class GP,
        class Vertex, class EdgeIndex >
    inline void save(Archive& ar,
        const boost::compressed_sparse_row_graph< Directed, VP, EP, GP, Vertex,
            EdgeIndex >& graph,
        const unsigned int /* version */)
    {
        const auto V = boost::num_vertices(graph);
        const auto E = boost::num_edges(graph);

        ar << BOOST_NVP(V);
        ar << BOOST_NVP(E);

        auto vertex_range = boost::vertices(graph);
        for (auto it = vertex_range.first; it != vertex_range.second; ++it)
        {
            const auto& vertex_property
                = boost::get(boost::vertex_all_t(), graph, *it);
            ar << BOOST_NVP(vertex_property);
        }

        auto edge_range = boost::edges(graph);
        for (auto it = edge_range.first; it != edge_range.second; ++it)
        {
            const auto u = boost::source(*it, graph);
            const auto v = boost::target(*it, graph);
            const auto& edge_property
                = boost::get(boost::edge_all_t(), graph, *it);

            ar << BOOST_NVP(u);
            ar << BOOST_NVP(v);
            ar << BOOST_NVP(edge_property);
        }

        ar << boost::make_nvp(
            "graph_property", boost::get_property(graph, boost::graph_all_t()));
    }

    template < class Archive, class Directed, class VP, class EP, class GP,
        class Vertex, class EdgeIndex >
    inline void load(Archive& ar,
        boost::compressed_sparse_row_graph< Directed, VP, EP, GP, Vertex,
            EdgeIndex >& graph,
        const unsigned int /* version */)
    {
        using Graph = boost::compressed_sparse_row_graph< Directed, VP, EP, GP,
            Vertex, EdgeIndex >;
        using VertexSize = typename Graph::vertices_size_type;
        using EdgeSize = typename Graph::edges_size_type;
        using VertexDescriptor = typename Graph::vertex_descriptor;
        using VertexProperty = typename Graph::vertex_bundled;
        using EdgeProperty = typename Graph::edge_bundled;

        VertexSize V = 0;
        EdgeSize E = 0;

        ar >> BOOST_NVP(V);
        ar >> BOOST_NVP(E);

        std::vector< VertexProperty > vertex_properties(
            static_cast< std::size_t >(V));
        for (auto& vertex_property : vertex_properties)
        {
            ar >> BOOST_NVP(vertex_property);
        }

        std::vector< std::pair< VertexDescriptor, VertexDescriptor > >
            endpoints;
        std::vector< EdgeProperty > edge_properties;
        endpoints.reserve(static_cast< std::size_t >(E));
        edge_properties.reserve(static_cast< std::size_t >(E));

        for (EdgeSize i = 0; i < E; ++i)
        {
            VertexDescriptor u = 0;
            VertexDescriptor v = 0;
            EdgeProperty edge_property { };

            ar >> BOOST_NVP(u);
            ar >> BOOST_NVP(v);
            ar >> BOOST_NVP(edge_property);

            endpoints.emplace_back(u, v);
            edge_properties.push_back(std::move(edge_property));
        }

        Graph rebuilt(boost::edges_are_sorted, endpoints.begin(),
            endpoints.end(), edge_properties.begin(), V, E);
        for (VertexDescriptor vertex = 0; vertex < V; ++vertex)
        {
            boost::get(boost::vertex_all_t(), rebuilt, vertex) = std::move(
                vertex_properties[static_cast< std::size_t >(vertex)]);
        }

        ar >> boost::make_nvp("graph_property",
            boost::get_property(rebuilt, boost::graph_all_t()));

        graph = std::move(rebuilt);
    }

    template < class Archive, class Directed, class VP, class EP, class GP,
        class Vertex, class EdgeIndex >
    inline void serialize(Archive& ar,
        boost::compressed_sparse_row_graph< Directed, VP, EP, GP, Vertex,
            EdgeIndex >& graph,
        const unsigned int version)
    {
        boost::core::split_free(ar, graph, version);
    }

} // namespace serialization
} // namespace boost

#endif // BOOST_GRAPH_CSR_GRAPH_SERIALIZE_HPP
