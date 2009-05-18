// (C) Copyright 2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_CONSTRUCTION_HPP
#define TEST_CONSTRUCTION_HPP

#include <utility>

/** @name Build Graph
 * Build the standard graph structure used in the remaining tests. Depending
 * on the mutability traits of the graph G, this may or may not add N vertices
 * to graph. The Add and Label type parameters are used to dispatch a build
 * method for normal or labeled graphs.
 */
//@{
// This will basically catch adjacency matrices, which don't get built.
template <typename Graph, typename Add, typename Label>
void build_graph(Graph& g, Add, Label)
{ }

// This matches MutableGraph, so just add some vertices.
template <typename Graph>
void build_graph(Graph& g, boost::mpl::true_, boost::mpl::false_) {
    for(std::size_t i = 0; i < N; ++i) {
        boost::add_vertex(g);
    }
}

// This will match labeled graphs.
template <typename Graph>
void build_graph(Graph& g, boost::mpl::false_, boost::mpl::true_) {
    // Add each vertex labeled with the number i.
    for(std::size_t i = 0; i < N; ++i) {
        boost::add_vertex(i, g);
    }
}
//@}


/** @name Connect Graph
 * Given a constructed graph, connect the edges to create a the standard
 * testing graph. To facilitate ease of use, we pass a vector of vertices
 * along with the graph such that v[0] -> *vertices(g).first, etc. The
 * Labeled type parameter is used to dispatch connection techniques for
 * normal or labled graphs.
 */
//@{
template <typename Graph, typename VertexSet>
void connect_graph(Graph& g, VertexSet const& verts, boost::mpl::false_) {
    Pair *f, *l;
    for(boost::tie(f, l) = edge_pairs(); f != l; ++f) {
        Pair const& e = *f;
        boost::add_edge(verts[e.first], verts[e.second], g);
    }

    // Is the lollipop connected? Is the lollipop not connected to the roof?
    BOOST_ASSERT(boost::edge(verts[5], verts[3], g).second == true);
    BOOST_ASSERT(boost::edge(verts[5], verts[0], g).second == false);
}

template <typename Graph, typename VertexSet>
void connect_graph(Graph& g, VertexSet const& verts, boost::mpl::true_) {
    // With labeled graphs, we want to operate directly on the edge numbers
    // rather than looking up the correct vertex index. This is because the
    // vertices are already mapped to indices.
    Pair* p = edge_pairs().first;
    for(std::size_t i = 0; i < M; ++i) {
        Pair const& e = p[i];
        boost::add_edge_by_label(e.first, e.second, g);
    }

    // Is the lollipop connected?
    BOOST_ASSERT(boost::edge_by_label(5, 3, g).second == true);
    BOOST_ASSERT(boost::edge_by_label(5, 0, g).second == false);
}
//@}

#endif
