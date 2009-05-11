
#ifndef TEST_ITERATION_HPP
#define TEST_ITERATION_HPP

#include <algorithm>

/** @name Test Vertex List
 * Test the vertex list interface. Note that there are currently no graphs that
 * do not expose this interface.
 */
//@{
template <typename Graph>
void test_vertex_list_graph(Graph const& g) {
    typedef typename boost::graph_traits<Graph>::vertex_iterator Iterator;
    typedef std::pair<Iterator, Iterator> Range;

    Range rng = vertices(g);
    BOOST_ASSERT(num_vertices(g) == N);
    BOOST_ASSERT(rng.first != rng.second);
    BOOST_ASSERT(std::distance(rng.first, rng.second) == int(N));
}
//@}

/** @name Test Edge List
 * Test the edge list interface. Note that there are currently no graphs that
 * do not expose this interface.
 */
//@{
template <typename Graph>
void test_edge_list_graph(Graph const& g) {
    typedef typename boost::graph_traits<Graph>::edge_iterator Iterator;
    typedef std::pair<Iterator, Iterator> Range;

    Range rng = edges(g);
    BOOST_ASSERT(boost::num_edges(g) == M);
    BOOST_ASSERT(rng.first != rng.second);
    BOOST_ASSERT(std::distance(rng.first, rng.second) == int(M));
}
//@}

#endif
