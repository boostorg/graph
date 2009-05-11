
#ifndef TEST_DESTRUCTION_HPP
#define TEST_DESTRUCTION_HPP

#include <utility>

/** @name Destroy Graph
 * Destroy the graph by removing vertices (if possible).
 */
//@{
// This will basically catch adjacency matrices, which don't get torn down.
template <typename Graph, typename VertexSet, typename Remove, typename Label>
void destroy_graph(Graph& g, VertexSet const& verts, Remove, Label)
{ }

// This matches MutableGraph, so just remove a vertex and then clear.
template <typename Graph, typename VertexSet>
void destroy_graph(Graph& g, VertexSet const& verts, boost::mpl::true_, boost::mpl::false_) {
    // Remove the roof vertex
    boost::remove_vertex(verts[0], g);
    BOOST_ASSERT(boost::num_vertices(g) == N - 1);
}

// This will match labeled graphs.
template <typename Graph, typename VertexSet>
void destroy_graph(Graph& g, VertexSet const& verts, boost::mpl::false_, boost::mpl::true_) {
    // Remove the roof vertex
    boost::remove_vertex(0, g);
    BOOST_ASSERT(boost::num_vertices(g) == N - 1);
}
//@}


/** @name Disconnect Graph
 * Disconnect edges in the graph. Note that this doesn't fully disconnect the
 * graph. It simply determines if we can disconnect an edge or two and verify
 * that the resulting graph is valid. The Labeled type parameter is used to
 * dispatch for unlabeled and labeled graphs.
 *
 * @todo This doesn't quite work for multigraphs...
 */
//@{

template <typename Graph, typename VertexSet>
void disconnect_graph(Graph& g, VertexSet const& verts, boost::mpl::false_) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;

    // Disconnect the "lollipop" from the house.
    Edge e = boost::edge(verts[5], verts[3], g).first;
    boost::remove_edge(e, g);
    BOOST_ASSERT(boost::num_edges(g) == M - 1);

    // Remove the "floor" edge from the house.
    boost::remove_edge(verts[3], verts[2], g);
    BOOST_ASSERT(boost::num_edges(g) == M - 2);

    // Fully disconnect the roof vertex.
    clear_vertex(verts[0], g);
    BOOST_ASSERT(boost::num_edges(g) == M - 4);

    // What happens if we try to remove an edge that doesn't exist?
    boost::remove_edge(verts[5], verts[0], g);
    BOOST_ASSERT(boost::num_edges(g) == M - 4);
}

template <typename Graph, typename VertexSet>
void disconnect_graph(Graph& g, VertexSet const& verts, boost::mpl::true_) {
    typedef typename boost::graph_traits<Graph>::edge_descriptor Edge;

    // Disconnect the "lollipop" from the house.
    Edge e = boost::edge_by_label(5, 3, g).first;
    boost::remove_edge(e, g);
    BOOST_ASSERT(boost::num_edges(g) == M - 1);

    // Remove the "floor" edge from the house.
    boost::remove_edge_by_label(3, 2, g);
    BOOST_ASSERT(boost::num_edges(g) == M - 2);

    // Fully disconnect the roof vertex.
    clear_vertex_by_label(0, g);
    BOOST_ASSERT(boost::num_edges(g) == M - 4);

    // What happens if we try to remove an edge that doesn't exist?
    boost::remove_edge_by_label(5, 0, g);
    BOOST_ASSERT(boost::num_edges(g) == M - 4);
}
//@}

#endif
