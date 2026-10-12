//=======================================================================
// Copyright 2005 Jeremy G. Siek
// Authors: Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_ADJ_LIST_SERIALIZE_HPP
#define BOOST_GRAPH_ADJ_LIST_SERIALIZE_HPP

#include <unordered_map>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/pending/property_serialize.hpp>
#include <boost/config.hpp>
#include <boost/detail/workaround.hpp>

#include <boost/core/serialization.hpp>
#include <boost/type_traits/integral_constant.hpp>

namespace boost
{

namespace serialization
{

    // Turn off tracking for adjacency_list. It's not polymorphic, and we
    // need to do this to enable saving of non-const adjacency lists.
    // Specializing tracking_level_impl (not tracking_level) avoids including
    // <boost/serialization/tracking.hpp>. boost::integral_constant carries the
    // integral_c_tag the archive expects.
    template < class T > struct tracking_level_impl;
    template < class OEL, class VL, class D, class VP, class EP, class GP,
        class EL >
    struct tracking_level_impl<
        const boost::adjacency_list< OEL, VL, D, VP, EP, GP, EL > >
    : boost::integral_constant< int, 0 /* track_never */ >
    {
    };

    template < class Archive, class OEL, class VL, class D, class VP, class EP,
        class GP, class EL >
    inline void save(Archive& ar,
        const boost::adjacency_list< OEL, VL, D, VP, EP, GP, EL >& graph,
        const unsigned int /* file_version */
    )
    {
        using Graph = adjacency_list< OEL, VL, D, VP, EP, GP, EL >;
        using Vertex = typename graph_traits< Graph >::vertex_descriptor;

        const auto V = num_vertices(graph);
        const auto E = num_edges(graph);
        ar << BOOST_NVP(V);
        ar << BOOST_NVP(E);

        // assign indices to vertices
        std::unordered_map< Vertex, Vertex > indices(V);
        Vertex num = 0;
        BGL_FORALL_VERTICES_T(v, graph, Graph)
        {
            indices[v] = num++;
            ar << boost::make_nvp(
                "vertex_property", get(vertex_all_t(), graph, v));
        }

        // write edges
        BGL_FORALL_EDGES_T(e, graph, Graph)
        {
            ar << boost::make_nvp("u", indices[source(e, graph)]);
            ar << boost::make_nvp("v", indices[target(e, graph)]);
            ar << boost::make_nvp(
                "edge_property", get(edge_all_t(), graph, e));
        }

        ar << boost::make_nvp(
            "graph_property", get_property(graph, graph_all_t()));
    }

    template < class Archive, class OEL, class VL, class D, class VP, class EP,
        class GP, class EL >
    inline void load(
        Archive& ar, boost::adjacency_list< OEL, VL, D, VP, EP, GP, EL >& graph,
        const unsigned int /* file_version */
    )
    {
        using Graph = adjacency_list< OEL, VL, D, VP, EP, GP, EL >;
        using Vertex = typename graph_traits< Graph >::vertex_descriptor;
        using Edge = typename graph_traits< Graph >::edge_descriptor;
        using VertexSizeType = typename graph_traits< Graph >::vertices_size_type;

        VertexSizeType V;
        ar >> BOOST_NVP(V);
        
        VertexSizeType E;
        ar >> BOOST_NVP(E);

        std::vector< Vertex > verts(V);
        size_t i = 0;
        while (V-- > 0)
        {
            const auto v = add_vertex(graph);
            verts[i++] = v;
            ar >> boost::make_nvp(
                "vertex_property", get(vertex_all_t(), graph, v));
        }
        
        while (E-- > 0)
        {
            Vertex u;
            Vertex v;
            ar >> BOOST_NVP(u);
            ar >> BOOST_NVP(v);

            Edge e;
            bool inserted;
            boost::tie(e, inserted) = add_edge(verts[u], verts[v], graph);
            ar >> boost::make_nvp(
                "edge_property", get(edge_all_t(), graph, e));
        }
        ar >> boost::make_nvp(
            "graph_property", get_property(graph, graph_all_t()));
    }

    template < class Archive, class OEL, class VL, class D, class VP, class EP,
        class GP, class EL >
    inline void serialize(Archive& ar,
        boost::adjacency_list< OEL, VL, D, VP, EP, GP, EL >& graph,
        const unsigned int file_version)
    {
        boost::core::split_free(ar, graph, file_version);
    }

} // serialization
} // boost

#endif // BOOST_GRAPH_ADJ_LIST_SERIALIZE_HPP
