// (C) Copyright 2007-2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <boost/graph/undirected_graph.hpp>

using namespace std;
using namespace boost;

typedef undirected_graph<> Graph;
typedef Graph::vertex_descriptor Vertex;
typedef property_map<Graph, vertex_index_t>::type IndexMap;

int main(int, char*[])
{
    static const size_t N = 5;
    Graph g;
    Vertex v[N];

    IndexMap x = get(vertex_index, g);

    // build up the graph
    for(size_t i = 0; i < N; ++i) {
        v[i] = add_vertex(g);
    }

    // after the first build, we should have these conditions
    BOOST_ASSERT(max_vertex_index(g) == N);
    for(size_t i = 0; i < N; ++i) {
        BOOST_ASSERT(get_vertex_index(v[i], g) == i);
    }

    // remove some vertices and re-add them...
    for(size_t i = 0; i < N; ++i) remove_vertex(v[i], g);
    BOOST_ASSERT(num_vertices(g) == 0);

    for(size_t i = 0; i < N; ++i) {
        v[i] = add_vertex(g);
    }

    // before renumbering, our vertices should be off by
    // about N...
    BOOST_ASSERT(max_vertex_index(g) == 10);
    for(size_t i = 0; i < N; ++i) {
        BOOST_ASSERT(get_vertex_index(v[i], g) == N + i);
    }

    // renumber vertices
    renumber_vertex_indices(g);

    // and we should be back to the initial condition
    BOOST_ASSERT(max_vertex_index(g) == N);
    for(size_t i = 0; i < N; ++i) {
        BOOST_ASSERT(get_vertex_index(v[i], g) == i);
    }

    return 0;
}
