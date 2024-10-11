//=======================================================================
// Copyright 2024
// Author: Hermann Stamm-Wilbrandt
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_PLANAR_VERTEX_SIX_COLORING_HPP
#define BOOST_GRAPH_PLANAR_VERTEX_SIX_COLORING_HPP

#include <vector>
#include <algorithm>
#include <boost/foreach.hpp>
#include <boost/graph/undirected_graph_constant_time_edge_add_and_remove.hpp>
#include <boost/graph/graph_utility.hpp>

/* This algorithm is to find a vertex six coloring of a planar graph

   Algorithm:
   Let G = (V,E) be a planar graph with n vertices.
   Planar graphs always have a vertex of degree 5 or less.
   Remember its (at most 5) adjacent vertices and remove its edges.
   Repeat until all vertices have been processed.
   Now color the vertices in reverse order they were removed.
   Assign smallest color different to all <=5 remembered vertices to vertex.
   Repeat until done.

   The color of the vertex v will be stored in color[v].
   i.e., vertex v belongs to coloring color[v] */

namespace boost
{
    enum vertex_back_t { vertex_back };
    BOOST_INSTALL_PROPERTY(vertex, back);

    enum vertex_adj5_t { vertex_adj5 };
    BOOST_INSTALL_PROPERTY(vertex, adj5);

template < class VertexListGraph, class ColorMap >
typename property_traits< ColorMap >::value_type planar_vertex_six_coloring(
    const VertexListGraph& G, ColorMap color)
{
    typedef typename VertexListGraph::vertex_descriptor vertex_descriptor;

    typedef typename property_traits< ColorMap >::value_type size_type;

    typedef undirected_graph_constant_time_edge_add_and_remove<
        property< vertex_back_t, vertex_descriptor,
            property< vertex_adj5_t, std::vector< vertex_descriptor > > >,
        no_property, no_property > ugraph_o1;


    std::vector< typename ugraph_o1::vertex_descriptor >
        vertex_u(num_vertices(G));
    auto vmap = make_container_vertex_map(vertex_u, G);
//
//                               get(vmap, _)
//                {v,w} in V(G)--------------->
//                             <---------------V(U) <= {u,t}
//                                 back[_]      |
//                                              |
//             std::vector<V(G)> <--------------+
//                                   adj5[_]
//
    ugraph_o1 U;

    auto adj5 = get(vertex_adj5, U);
    auto back = get(vertex_back, U);


    BGL_FORALL_VERTICES_T(v, G, VertexListGraph)
        { put(vmap, v, U.add_vertex(v)); }

    BGL_FORALL_EDGES_T(e, G, VertexListGraph)
        { U.add_edge(get(vmap, source(e, G)), get(vmap, target(e, G))); }


    std::vector< typename ugraph_o1::vertex_descriptor > small, visited;
    small.reserve(num_vertices(U));
    visited.reserve(num_vertices(U));

    BGL_FORALL_VERTICES_T(u, U, ugraph_o1)
        { adj5[u].reserve(5); if (degree(u, U) <= 5) { small.push_back(u); } }


    while (!small.empty())
    {
        typename ugraph_o1::vertex_descriptor u = small.back();
        small.pop_back();

        BGL_FORALL_ADJ_T(u, t, U, ugraph_o1)
        {
            adj5[u].push_back(back[t]);
            if (degree(t, U) == 6)  small.push_back(t);
        }
        U.clear_vertex(u);

        visited.push_back(u);
    }

    BOOST_ASSERT(num_vertices(U) == visited.size());


    // highest possisble index with <=5 neighbors is 0x3E (= 0b111110)
    const size_type smallest_possible_color[1+0x3E]
        = {0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
           0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 5,
           0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0, 4,
           0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0};

    size_type colmax = 0;
    typename ugraph_o1::vertex_descriptor u;
    BOOST_REVERSE_FOREACH(u, visited)
    {
        size_type bs = 0;
        vertex_descriptor w;
        BOOST_FOREACH(w, adj5[u]) { bs |= (1 << get(color, w)); }
        put(color, back[u], smallest_possible_color[bs]);
        colmax = std::max BOOST_PREVENT_MACRO_SUBSTITUTION(
                              colmax, smallest_possible_color[bs]);
    }

    return colmax+1;
}

}  // namespace boost

#endif  // BOOST_GRAPH_PLANAR_VERTEX_SIX_COLORING_HPP
