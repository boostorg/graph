//  (C) Copyright Jeremy Siek 2004
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// From Louis Lavery <Louis@devilsChimney.co.uk>
/*Expected Output:-
A:   0 A
B:  11 A

Actual Output:-
A:   0 A
B: 2147483647 B
*/

#include <iostream>
#include <iomanip>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/cstdlib.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>
#include <limits>
#include <vector>

// state in a plain data member, so it survives only through std::ref
struct relaxed_tally : boost::bellman_visitor<>
{
    template < class Edge, class Graph > void edge_relaxed(Edge, Graph&)
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

    // bellman_ford does not initialise, the caller seeds the distances
    std::vector< std::size_t > parent(boost::num_vertices(g));
    for (std::size_t i = 0; i < parent.size(); ++i)
        parent[i] = i;
    std::vector< int > distance(
        boost::num_vertices(g), (std::numeric_limits< int >::max)());
    distance[0] = 0;

    auto index_map = boost::get(boost::vertex_index, g);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);

    relaxed_tally tracked;
    BOOST_TEST(boost::bellman_ford_shortest_paths(g,
        static_cast< int >(boost::num_vertices(g)),
        boost::get(boost::edge_weight, g), parent_map, distance_map,
        boost::closed_plus< int >(), std::less< int >(), std::ref(tracked)));
    BOOST_TEST(tracked.count > 0u);
    BOOST_TEST_EQ(distance[2], 2);

    // by value the caller's visitor is left untouched
    relaxed_tally copied;
    BOOST_TEST(boost::bellman_ford_shortest_paths(g,
        static_cast< int >(boost::num_vertices(g)),
        boost::get(boost::edge_weight, g), parent_map, distance_map,
        boost::closed_plus< int >(), std::less< int >(), copied));
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

int main(int, char*[])
{
    using namespace boost;

    enum
    {
        A,
        B,
        Z
    };
    char const name[] = "ABZ";
    int const numVertex = static_cast< int >(Z) + 1;
    typedef std::pair< int, int > Edge;
    Edge edge_array[] = { Edge(B, A) };
    int const numEdges = sizeof(edge_array) / sizeof(Edge);
    int const weight[numEdges] = { 11 };

    typedef adjacency_list< vecS, vecS, undirectedS, no_property,
        property< edge_weight_t, int > >
        Graph;

    Graph g(edge_array, edge_array + numEdges, numVertex);

    Graph::edge_iterator ei, ei_end;
    property_map< Graph, edge_weight_t >::type weight_pmap
        = get(edge_weight, g);

    int i = 0;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei, ++i)
        weight_pmap[*ei] = weight[i];

    std::vector< int > parent(numVertex);
    for (i = 0; i < numVertex; ++i)
        parent[i] = i;

    int inf = (std::numeric_limits< int >::max)();
    std::vector< int > distance(numVertex, inf);
    distance[A] = 0; // Set source distance to zero

    bool const r = bellman_ford_shortest_paths(g, int(numVertex), weight_pmap,
        boost::make_iterator_property_map(
            parent.begin(), get(boost::vertex_index, g)),
        boost::make_iterator_property_map(
            distance.begin(), get(boost::vertex_index, g)),
        closed_plus< int >(), std::less< int >(), default_bellman_visitor());

    if (r)
    {
        for (int i = 0; i < numVertex; ++i)
        {
            std::cout << name[i] << ": ";
            if (distance[i] == inf)
                std::cout << std::setw(3) << "inf";
            else
                std::cout << std::setw(3) << distance[i];
            std::cout << " " << name[parent[i]] << std::endl;
        }
    }
    else
    {
        std::cout << "negative cycle" << std::endl;
    }

#if !(defined(__INTEL_COMPILER) && __INTEL_COMPILER <= 700) \
    && !(defined(BOOST_MSVC) && BOOST_MSVC <= 1300)
    graph_traits< Graph >::vertex_descriptor s = vertex(A, g);
    std::vector< int > parent2(numVertex);
    std::vector< int > distance2(numVertex, 17);
    bool const r2 = bellman_ford_shortest_paths(g,
        weight_map(weight_pmap)
            .distance_map(boost::make_iterator_property_map(
                distance2.begin(), get(boost::vertex_index, g)))
            .predecessor_map(boost::make_iterator_property_map(
                parent2.begin(), get(boost::vertex_index, g)))
            .root_vertex(s));
    if (r2)
    {
        for (int i = 0; i < numVertex; ++i)
        {
            std::cout << name[i] << ": ";
            if (distance2[i] == inf)
                std::cout << std::setw(3) << "inf";
            else
                std::cout << std::setw(3) << distance2[i];
            std::cout << " " << name[parent2[i]] << std::endl;
        }
    }
    else
    {
        std::cout << "negative cycle" << std::endl;
    }

    BOOST_TEST(r == r2);
    if (r && r2)
    {
        BOOST_TEST(parent == parent2);
        BOOST_TEST(distance == distance2);
    }
#endif

    test_stateful_visitor_with_ref();

    return boost::report_errors();
}
