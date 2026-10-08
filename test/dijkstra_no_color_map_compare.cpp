//=======================================================================
// Copyright 2009 Trustees of Indiana University.
// Authors: Michael Hansen
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <map>
#include <vector>
#include <boost/config.hpp>

#ifdef BOOST_MSVC
// Without disabling this we get hard errors about initialialized pointers:
#pragma warning(disable : 4703)
#endif

#include <boost/lexical_cast.hpp>
#include <boost/random.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/random.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/iteration_macros.hpp>

#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

#define INITIALIZE_VERTEX 0
#define DISCOVER_VERTEX 1
#define EXAMINE_VERTEX 2
#define EXAMINE_EDGE 3
#define EDGE_RELAXED 4
#define EDGE_NOT_RELAXED 5
#define FINISH_VERTEX 6

template < typename Graph > void run_dijkstra_test(const Graph& graph)
{
    using namespace boost;

    // Set up property maps
    typedef typename graph_traits< Graph >::vertex_descriptor vertex_t;

    typedef typename std::map< vertex_t, vertex_t > vertex_map_t;
    typedef associative_property_map< vertex_map_t > predecessor_map_t;
    vertex_map_t default_vertex_map, no_color_map_vertex_map;
    predecessor_map_t default_predecessor_map(default_vertex_map),
        no_color_map_predecessor_map(no_color_map_vertex_map);

    typedef typename std::map< vertex_t, double > vertex_double_map_t;
    typedef associative_property_map< vertex_double_map_t > distance_map_t;
    vertex_double_map_t default_vertex_double_map,
        no_color_map_vertex_double_map;
    distance_map_t default_distance_map(default_vertex_double_map),
        no_color_map_distance_map(no_color_map_vertex_double_map);

    // Run dijkstra algoirthms
    dijkstra_shortest_paths(graph, vertex(0, graph),
        predecessor_map(default_predecessor_map)
            .distance_map(default_distance_map));

    dijkstra_shortest_paths_no_color_map(graph, vertex(0, graph),
        predecessor_map(no_color_map_predecessor_map)
            .distance_map(no_color_map_distance_map));

    // Verify that predecessor maps are equal
    BOOST_TEST(std::equal(default_vertex_map.begin(), default_vertex_map.end(),
        no_color_map_vertex_map.begin()));

    // Verify that distance maps are equal
    BOOST_TEST(std::equal(default_vertex_double_map.begin(),
        default_vertex_double_map.end(),
        no_color_map_vertex_double_map.begin()));
}

// state in a plain data member, so it survives only through std::ref
struct discover_tally : boost::dijkstra_visitor<>
{
    template < class Vertex, class Graph > void discover_vertex(Vertex, Graph&)
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

    auto index_map = boost::get(boost::vertex_index, g);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);

    discover_tally tracked;
    boost::dijkstra_shortest_paths_no_color_map(g, boost::vertex(0, g),
        parent_map, distance_map, boost::get(boost::edge_weight, g), index_map,
        std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0, std::ref(tracked));
    BOOST_TEST(tracked.count > 0u);
    BOOST_TEST_EQ(distance[2], 2);

    // by value the caller's visitor is left untouched
    discover_tally copied;
    boost::dijkstra_shortest_paths_no_color_map(g, boost::vertex(0, g),
        parent_map, distance_map, boost::get(boost::edge_weight, g), index_map,
        std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0, copied);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

int main(int argc, char* argv[])
{
    using namespace boost;

    int vertices_to_create = 10;
    int edges_to_create = 500;
    std::size_t random_seed = 42;

    if (argc > 1)
    {
        vertices_to_create = lexical_cast< int >(argv[1]);
    }

    if (argc > 2)
    {
        edges_to_create = lexical_cast< int >(argv[2]);
    }

    minstd_rand generator(random_seed);

    // Set up graph
    typedef adjacency_list< listS, listS, directedS,
        property< vertex_index_t, int >, property< edge_weight_t, double > >
        graph_t;

    graph_t graph;
    generate_random_graph(
        graph, vertices_to_create, edges_to_create, generator);

    // Set up property maps
    typedef property_map< graph_t, vertex_index_t >::type index_map_t;
    index_map_t index_map = get(vertex_index, graph);
    int vertex_index = 0;

    BGL_FORALL_VERTICES(current_vertex, graph, graph_t)
    {
        put(index_map, current_vertex, vertex_index++);
    }

    randomize_property< edge_weight_t >(graph, generator);

    // Run comparison test with original dijkstra_shortest_paths
    std::cout << "Running dijkstra shortest paths test with "
              << num_vertices(graph) << " vertices and " << num_edges(graph)
              << " edges " << std::endl;

    run_dijkstra_test(graph);

    test_stateful_visitor_with_ref();

    return boost::report_errors();
}
