// (C) Copyright David Gleich 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/core_numbers.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>
#include <stdio.h>

using namespace boost;

const char* errstr = "";

int test_1()
{
    // core numbers of sample graph
    typedef adjacency_list< vecS, vecS, undirectedS > Graph;

    Graph G(21);
    add_edge(0, 1, G);
    add_edge(1, 2, G);
    add_edge(1, 3, G);
    add_edge(2, 3, G);
    add_edge(1, 4, G);
    add_edge(3, 4, G);
    add_edge(4, 5, G);
    add_edge(4, 6, G);
    add_edge(5, 6, G);
    add_edge(4, 7, G);
    add_edge(5, 7, G);
    add_edge(6, 7, G);
    add_edge(7, 8, G);
    add_edge(3, 9, G);
    add_edge(8, 9, G);
    add_edge(8, 10, G);
    add_edge(9, 10, G);
    add_edge(10, 11, G);
    add_edge(10, 12, G);
    add_edge(3, 13, G);
    add_edge(9, 13, G);
    add_edge(3, 14, G);
    add_edge(9, 14, G);
    add_edge(13, 14, G);
    add_edge(16, 17, G);
    add_edge(16, 18, G);
    add_edge(17, 19, G);
    add_edge(18, 19, G);
    add_edge(19, 20, G);

    std::vector< int > core_nums(num_vertices(G));
    core_numbers(
        G, make_iterator_property_map(core_nums.begin(), get(vertex_index, G)));

    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        printf("vertex %3lu : %i\n", (unsigned long)i, core_nums[i]);
    }

    int correct[21]
        = { 1, 2, 2, 3, 3, 3, 3, 3, 2, 3, 2, 1, 1, 3, 3, 0, 2, 2, 2, 2, 1 };
    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        if (core_nums[i] != correct[i])
        {
            return 1; // error!
        }
    }
    return 0;
}

int test_2()
{
    // core numbers of sample graph
    typedef adjacency_list< listS, vecS, undirectedS, no_property,
        property< edge_weight_t, int > >
        graph_t;
    int num_nodes = 3;
    typedef std::pair< int, int > Edge;

    Edge edge_array[] = { Edge(0, 1), Edge(0, 2), Edge(1, 2) };
    int weights[] = { -1, -2, -2 };
    int num_arcs = sizeof(edge_array) / sizeof(Edge);

    graph_t G(edge_array, edge_array + num_arcs, weights, num_nodes);

    std::vector< int > core_nums(num_vertices(G));
    weighted_core_numbers(
        G, make_iterator_property_map(core_nums.begin(), get(vertex_index, G)));

    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        printf("vertex %3lu : %i\n", (unsigned long)i, core_nums[i]);
    }

    int correct[3] = { -1, -1, -4 };
    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        if (core_nums[i] != correct[i])
        {
            return 1; // error!
        }
    }
    return 0;
}

int test_3()
{
    // core numbers of a directed graph, the core numbers of a directed
    // cycle are always one
    typedef adjacency_list< vecS, vecS, directedS > graph_t;
    int num_nodes = 5;
    typedef std::pair< int, int > Edge;

    Edge edge_array[]
        = { Edge(0, 1), Edge(1, 2), Edge(2, 3), Edge(3, 4), Edge(4, 0) };
    int num_arcs = sizeof(edge_array) / sizeof(Edge);

    graph_t G(edge_array, edge_array + num_arcs, num_nodes);

    std::vector< int > core_nums(num_vertices(G));
    core_numbers(
        G, make_iterator_property_map(core_nums.begin(), get(vertex_index, G)));

    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        printf("vertex %3lu : %i\n", (unsigned long)i, core_nums[i]);
    }

    int correct[5] = { 1, 1, 1, 1, 1 };
    for (size_t i = 0; i < num_vertices(G); ++i)
    {
        if (core_nums[i] != correct[i])
        {
            return 1; // error!
        }
    }
    return 0;
}

// state in a plain data member, so it survives only through std::ref
struct examine_tally : boost::core_numbers_visitor<>
{
    template < typename Vertex, typename Graph >
    void examine_vertex(Vertex, Graph&)
    {
        ++count;
    }
    std::size_t count = 0;
};

void test_stateful_visitor_with_ref()
{
    using graph_t
        = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >;

    // a triangle with a pendant vertex, every vertex is examined once
    graph_t g(4);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 0, g);
    boost::add_edge(2, 3, g);

    std::vector< int > core(boost::num_vertices(g));
    auto core_map = boost::make_iterator_property_map(
        core.begin(), boost::get(boost::vertex_index, g));

    examine_tally tracked;
    boost::core_numbers(g, core_map, std::ref(tracked));
    BOOST_TEST_EQ(tracked.count, boost::num_vertices(g));

    // by value the caller's visitor is left untouched
    examine_tally copied;
    boost::core_numbers(g, core_map, copied);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

int main(int, char**)
{
    int nfail = 0, ntotal = 0;
    int rval;

    const char* name;

    name = "core_numbers";
    rval = test_1();
    ntotal++;
    if (rval != 0)
    {
        nfail++;
        printf("%20s  %50s\n", name, errstr);
    }
    else
    {
        printf("%20s  success\n", name);
    }

    name = "weighted_core_numbers";
    rval = test_2();
    ntotal++;
    if (rval != 0)
    {
        nfail++;
        printf("%20s  %50s\n", name, errstr);
    }
    else
    {
        printf("%20s  success\n", name);
    }

    name = "directed_corenums";
    rval = test_3();
    ntotal++;
    if (rval != 0)
    {
        nfail++;
        printf("%20s  %50s\n", name, errstr);
    }
    else
    {
        printf("%20s  success\n", name);
    }

    printf("\n");
    printf("Total tests  : %3i\n", ntotal);
    printf("Total failed : %3i\n", nfail);

    test_stateful_visitor_with_ref();

    return (boost::report_errors() != 0) || (nfail != 0);
}
