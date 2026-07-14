//  Copyright (c) 2026 Arnaud Becheler
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>

#include <utility>

using namespace boost;

template < typename Directedness >
using subgraph_of = subgraph< adjacency_list< vecS, vecS, Directedness,
    property< vertex_color_t, int >,
    property< edge_index_t, std::size_t, property< edge_weight_t, int > > > >;

template < typename Directedness >
void test_descriptor_conversion()
{
    using graph_t = subgraph_of< Directedness >;
    using vertex_t = typename graph_traits< graph_t >::vertex_descriptor;
    using edge_t = typename graph_traits< graph_t >::edge_descriptor;
    using vertex_iterator = typename graph_traits< graph_t >::vertex_iterator;
    using edge_iterator = typename graph_traits< graph_t >::edge_iterator;

    // root: path 0-1-2-3, with edge indices assigned in insertion order.
    graph_t root(4);
    add_edge(0, 1, root); // edge index 0
    add_edge(1, 2, root); // edge index 1
    add_edge(2, 3, root); // edge index 2

    // child g1 = {1, 2, 3}: induces edges (1,2) and (2,3), but not (0,1).
    graph_t& g1 = root.create_subgraph();
    add_vertex(1, g1);
    add_vertex(2, g1);
    add_vertex(3, g1);

    // grandchild g1a = {2, 3}: induces only edge (2,3).
    graph_t& g1a = g1.create_subgraph();
    add_vertex(2, g1a);
    add_vertex(3, g1a);

    // invariant: for an edge in the subgraph, global_to_local(local_to_global(e))
    // must equal e, and its edge_index must be preserved.
    edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g1); ei != ei_end; ++ei)
    {
        edge_t e_local = *ei;
        edge_t e_round = g1.global_to_local(g1.local_to_global(e_local));
        BOOST_TEST(e_round == e_local);
        BOOST_TEST(get(edge_index, g1, e_local) == get(edge_index, g1, e_round));
    }

    // invariant: for a vertex in the subgraph, global_to_local(local_to_global(v))
    // must equal v, and its vertex_index must be preserved.
    vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g1); vi != vi_end; ++vi)
    {
        vertex_t v_local = *vi;
        vertex_t v_round = g1.global_to_local(g1.local_to_global(v_local));
        BOOST_TEST(v_round == v_local);
        BOOST_TEST(get(vertex_index, g1, v_local) == get(vertex_index, g1, v_round));
    }

    // invariant: a property written through a child must be readable from the
    // root, because the whole tree shares one property store.
    edge_t some_local = *edges(g1).first;
    put(edge_weight, g1, some_local, 42);
    BOOST_TEST(get(edge_weight, root, g1.local_to_global(some_local)) == 42);

    // invariant: find_vertex must report a present vertex as true, and an absent
    // one as false with a null_vertex() descriptor.
    BOOST_TEST(g1.find_vertex(1).second); // vertex 1 is in g1
    BOOST_TEST(!g1.find_vertex(0).second); // vertex 0 is not in g1
    BOOST_TEST(g1.find_vertex(0).first == graph_traits< graph_t >::null_vertex());

    // invariant: find_edge must report false with a default descriptor for a
    // root edge that is absent from the subgraph.
    edge_t e01 = edge(0, 1, root).first;
    BOOST_TEST(get(edge_index, root, e01) == 0u);
    BOOST_TEST(g1.find_edge(e01).first == edge_t()); // absent -> default
    BOOST_TEST(!g1.find_edge(e01).second); // absent -> false

    // invariant: edge round-trip identity must hold at any nesting depth, 
    // not just for direct children.
    for (boost::tie(ei, ei_end) = edges(g1a); ei != ei_end; ++ei)
    {
        edge_t e_local = *ei;
        BOOST_TEST(g1a.global_to_local(g1a.local_to_global(e_local)) == e_local);
    }

    // invariant: a property written through a grandchild must reach the root.
    edge_t g1a_local = *edges(g1a).first;
    put(edge_weight, g1a, g1a_local, 7);
    BOOST_TEST(get(edge_weight, root, g1a.local_to_global(g1a_local)) == 7);

    // invariant: an edge present in the parent but not the grandchild must
    // report absent from the grandchild.
    edge_t e12 = edge(1, 2, root).first;
    BOOST_TEST(!g1a.find_edge(e12).second);
}

// invariant: global_to_local on an edge that is not in the subgraph must return
// a default-constructed descriptor rather than dereferencing a missing map entry.
template < typename Directedness >
void test_absent_edge_returns_default()
{
    using graph_t = subgraph_of< Directedness >;
    using edge_t = typename graph_traits< graph_t >::edge_descriptor;

    graph_t root(3);
    add_edge(0, 1, root); // edge index 0
    add_edge(1, 2, root); // edge index 1

    // sg = {1, 2}: induces edge (1,2), but not edge (0,1).
    graph_t& sg = root.create_subgraph();
    add_vertex(1, sg);
    add_vertex(2, sg);

    edge_t e01 = edge(0, 1, root).first; // in root, absent from sg
    BOOST_TEST(sg.global_to_local(e01) == edge_t());
}

int main()
{
    test_descriptor_conversion< directedS >();
    test_descriptor_conversion< bidirectionalS >();
    test_absent_edge_returns_default< directedS >();
    test_absent_edge_returns_default< bidirectionalS >();
    return boost::report_errors();
}
