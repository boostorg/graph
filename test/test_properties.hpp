// (C) Copyright 2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#ifndef TEST_PROPERTIES_HPP
#define TEST_PROPERTIES_HPP

/** @name Test Vertex Bundle
 * Exercise the vertex bundle. Note that this is expected to be of type
 * VertexBundle.
 */
//@{
template <typename Graph, typename VertexSet>
void test_vertex_bundle(Graph& g, VertexSet const& verts, boost::mpl::true_) {
    std::cout << "...test_vertex_bundle\n";
    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;

    // This just has to compile. You can't actually get this map.
    typedef typename boost::property_map<Graph, boost::vertex_bundle_t>::type TestMap;

    // Test bundling via the graph object on the lollipop vertex.
    Vertex v = verts[5];
    VertexBundle& b = g[v];
    b.value = 10;
    BOOST_ASSERT(g[v].value == 10);

    // Test bundling via the property map.
    typedef typename boost::property_map<Graph, int VertexBundle::*>::type BundleMap;
    BundleMap map = get(&VertexBundle::value, g);
    put(map, v, 5);
    BOOST_ASSERT(get(map, v) == 5);
}

template <typename Graph, typename VertexSet>
void test_vertex_bundle(Graph&, VertexSet const&, boost::mpl::false_)
{ }
//@}

/** @name Test Edge Bundle
 * Exercise the edge bundle. Note that this is expected to be of type
 * EdgeBundle.
 */
//@{
template <typename Graph, typename VertexSet>
void test_edge_bundle(Graph& g, VertexSet const& verts, boost::mpl::true_) {
    std::cout << "...test_edge_bundle\n";
    // This just has to compile. You can't actually get this map.
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;
    typedef typename boost::property_map<Graph, boost::edge_bundle_t>::type TestMap;

    // Test bundling via the graph object on the lollipop edge.
    Edge e = boost::edge(verts[5], verts[3], g).first;
    EdgeBundle& b = g[e];
    b.value = 10;
    BOOST_ASSERT(g[e].value == 10);

    // Test bundling via the property map.
    typedef typename boost::property_map<Graph, int EdgeBundle::*>::type BundleMap;
    BundleMap map = get(&EdgeBundle::value, g);
    put(map, e, 5);
    BOOST_ASSERT(get(map, e) == 5);
}

template <typename Graph, typename VertexSet>
void test_edge_bundle(Graph&, VertexSet const&, boost::mpl::false_)
{ }
//@}

/**
 * Test the properties of a graph. Basically, we expect these to be one of
 * bundled or not. This test could also be expanded to test non-bundled
 * properties. This just bootstraps the tests.
 */
template <typename Graph, typename VertexSet>
void test_properties(Graph& g, VertexSet const& verts) {
    typename boost::has_bundled_vertex_property<Graph>::type vertex_bundled;
    typename boost::has_bundled_edge_property<Graph>::type edge_bundled;

    test_vertex_bundle(g, verts, vertex_bundled);
    test_edge_bundle(g, verts, edge_bundled);
}
//@}

#endif
