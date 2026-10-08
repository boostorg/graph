// Copyright (C) 2002 Trustees of Indiana University

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/core/lightweight_test.hpp>

#include <algorithm>
#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

using namespace boost;

#include <iostream>
using namespace std;

// state in a plain data member, so it survives only through std::ref
struct examine_tally : boost::dijkstra_visitor<>
{
    template < class Vertex, class Graph > void examine_vertex(Vertex, Graph&)
    {
        ++count;
    }
    std::size_t count = 0;
};

void test_stateful_visitor_with_ref()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::directedS, boost::no_property,
        boost::property< boost::edge_weight_t, int > >;
    graph_t g(3);
    boost::add_edge(0, 1, 1, g);
    boost::add_edge(1, 2, 1, g);

    std::vector< int > distance(boost::num_vertices(g));
    std::vector< std::size_t > parent(boost::num_vertices(g));
    std::vector< boost::default_color_type > color(boost::num_vertices(g));

    auto index_map = boost::get(boost::vertex_index, g);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    // every vertex is reachable from 0, so each one is examined once
    examine_tally tracked;
    boost::dag_shortest_paths(g, 0, distance_map,
        boost::get(boost::edge_weight, g), color_map, parent_map,
        std::ref(tracked), std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);
    BOOST_TEST_EQ(tracked.count, boost::num_vertices(g));
    BOOST_TEST_EQ(distance[2], 2);

    // by value the caller's visitor is left untouched
    std::fill(color.begin(), color.end(), boost::white_color);
    examine_tally copied;
    boost::dag_shortest_paths(g, 0, distance_map,
        boost::get(boost::edge_weight, g), color_map, parent_map, copied,
        std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

int main(int, char*[])
{
    typedef adjacency_list< vecS, vecS, directedS, no_property,
        property< edge_weight_t, int > >
        Graph;

    Graph graph;

    (void)add_vertex(graph);
    (void)add_vertex(graph);
    (void)add_vertex(graph);
    (void)add_vertex(graph);

    Graph::edge_descriptor e;

    e = add_edge(0, 1, graph).first;
    put(edge_weight, graph, e, 1);

    e = add_edge(1, 2, graph).first;
    put(edge_weight, graph, e, 1);

    e = add_edge(3, 1, graph).first;
    put(edge_weight, graph, e, 5);

    vector_property_map< int > distance;

    dag_shortest_paths(graph, 0,
        distance_map(distance)
            .distance_compare(std::greater< int >())
            .distance_inf((std::numeric_limits< int >::min)())
            .distance_zero(0));

    cout << distance[2] << "\n";

    BOOST_TEST(distance[2] == 2);

    test_stateful_visitor_with_ref();

    return boost::report_errors();
}
