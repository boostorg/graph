//=======================================================================
// Copyright (C) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/neighbor_bfs.hpp>
#include <boost/pending/queue.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <vector>

// state in a plain data member, so it survives only through std::ref
struct discover_tally : boost::neighbor_bfs_visitor<>
{
    template < class Vertex, class Graph > void discover_vertex(Vertex, Graph&)
    {
        ++count;
    }
    std::size_t count = 0;
};

int main()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::bidirectionalS >;
    using vertex_t = boost::graph_traits< graph_t >::vertex_descriptor;

    // vertex 3 is reachable from 0 through an in edge only, which is what
    // separates this search from a plain breadth first one
    graph_t g(4);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(3, 0, g);

    std::vector< boost::default_color_type > color(boost::num_vertices(g));
    auto index_map = boost::get(boost::vertex_index, g);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    discover_tally tracked;
    boost::queue< vertex_t > tracked_queue;
    boost::neighbor_breadth_first_search(
        g, 0, tracked_queue, std::ref(tracked), color_map);
    BOOST_TEST_EQ(tracked.count, boost::num_vertices(g));

    // the visit overload never initialises, the caller paints the colours
    std::fill(color.begin(), color.end(), boost::white_color);
    discover_tally visited;
    boost::queue< vertex_t > visit_queue;
    boost::neighbor_breadth_first_visit(
        g, 0, visit_queue, std::ref(visited), color_map);
    BOOST_TEST_EQ(visited.count, boost::num_vertices(g));

    // by value the caller's visitor is left untouched
    discover_tally copied;
    boost::queue< vertex_t > copied_queue;
    boost::neighbor_breadth_first_search(
        g, 0, copied_queue, copied, color_map);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));

    return boost::report_errors();
}
