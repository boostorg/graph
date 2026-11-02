//=======================================================================
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <cstddef>

using Graph = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, ::boost::undirectedS >;
using Vertex = ::boost::graph_traits< Graph >::vertex_descriptor;

// Hides vertex 0 and therefore its edges.
struct hide_vertex_zero
{
    bool operator()(Vertex v) const { return v != 0; }
};

using FilteredGraph = ::boost::filtered_graph< Graph, ::boost::keep_all, hide_vertex_zero >;

// Stops the search at the first mapping.
struct stop_at_first_mapping
{
    template < typename Map1To2, typename Map2To1 >
    bool operator()(Map1To2, Map2To1) const
    {
        return false;
    }
};

// Builds a triangle on vertices 1 2 3 with vertex 0 attached to vertex 1.
Graph make_base_graph()
{
    Graph g(4);
    ::boost::add_edge(1, 2, g);
    ::boost::add_edge(2, 3, g);
    ::boost::add_edge(3, 1, g);
    ::boost::add_edge(0, 1, g);
    return g;
}

// Builds the complete graph on n vertices.
Graph make_complete_graph(std::size_t n)
{
    Graph g(n);
    for (std::size_t u = 0; u < n; ++u)
    {
        for (std::size_t v = u + 1; v < n; ++v)
        {
            ::boost::add_edge(u, v, g);
        }
    }
    return g;
}

// Checks that the filtered triangle is found inside the complete graph on 4 vertices.
void test_filtered_subgraph_iso()
{
    const Graph base = make_base_graph();
    const FilteredGraph g(base, ::boost::keep_all(), hide_vertex_zero());
    const Graph large = make_complete_graph(4);

    BOOST_TEST(::boost::vf2_subgraph_iso(g, large, stop_at_first_mapping()));
}

// Checks that the filtered triangle is isomorphic to a triangle.
void test_filtered_graph_iso()
{
    const Graph base = make_base_graph();
    const FilteredGraph g(base, ::boost::keep_all(), hide_vertex_zero());
    const Graph triangle = make_complete_graph(3);

    BOOST_TEST(::boost::vf2_graph_iso(g, triangle, stop_at_first_mapping()));
}

int main()
{
    test_filtered_subgraph_iso();
    test_filtered_graph_iso();
    return ::boost::report_errors();
}
