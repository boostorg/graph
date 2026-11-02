//            Copyright Daniel Trebbien 2010.
// Copyright Arnaud Becheler 2026.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)

// Exercises the positional boost::graph::stoer_wagner_min_cut interface.
// The deprecated named parameter forms are covered in stoer_wagner_test_old.cpp.

#include <fstream>
#include <functional>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <boost/core/lightweight_test.hpp>

#include "mas_sw_maxflow_oracle.hpp"

using mas_sw_oracle::undirected_graph;
using mas_sw_oracle::weight_map_type;
using mas_sw_oracle::weight_type;
using mas_sw_oracle::vertex_descriptor;
using mas_sw_oracle::edge_descriptor;

using undirected_unweighted_graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::undirectedS >;

std::string test_dir;

struct edge_t
{
    unsigned long first;
    unsigned long second;
};

// Build the keyed max priority queue the Stoer-Wagner phases run on.
template < class Graph >
auto make_maxheap(const Graph& g)
{
    using vd = typename boost::graph_traits< Graph >::vertex_descriptor;
    using index_map_type = typename boost::property_map< Graph, boost::vertex_index_t >::const_type;
    using distances_type = boost::shared_array_property_map< weight_type, index_map_type >;
    using index_in_heap_type = typename std::vector< vd >::size_type;
    using indices_type = boost::shared_array_property_map< index_in_heap_type, index_map_type >;
    using maxheap_type = boost::d_ary_heap_indirect< vd, 22, indices_type, distances_type, std::greater< weight_type > >;

    const index_map_type index_map = get(boost::vertex_index, g);
    distances_type distances_map = boost::make_shared_array_property_map(num_vertices(g), static_cast< weight_type >(0), index_map);
    indices_type indices_map = boost::make_shared_array_property_map(num_vertices(g), static_cast< index_in_heap_type >(-1), index_map);
    return maxheap_type(distances_map, indices_map);
}

// Assignment work map: each vertex maps to its supernode representative.
template < class Graph >
auto make_assignment_map(const Graph& g)
{
    using vd = typename boost::graph_traits< Graph >::vertex_descriptor;
    return boost::make_shared_array_property_map(num_vertices(g), vd(), get(boost::vertex_index, g));
}

// Stoer Wagner global min cut on a curated set, checked against the known value.
void test_curated_min_cuts()
{
    for (const mas_sw_oracle::curated_graph& cg : mas_sw_oracle::curated_graphs())
    {
        const undirected_graph& g = cg.graph;
        BOOST_TEST(mas_sw_oracle::is_connected(g));
        const weight_type w = boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map());
        BOOST_TEST_EQ(w, cg.min_cut);
    }
}

// the example from Stoer & Wagner (1997), via the convenience overload
void test0()
{
    edge_t edges[] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 4 }, { 1, 4 },
        { 1, 5 }, { 2, 6 }, { 3, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 } };
    weight_type ws[] = { 2, 3, 4, 3, 2, 2, 2, 2, 2, 3, 1, 3 };
    undirected_graph g(edges, edges + 12, ws, 8, 12);

    weight_map_type weights = get(boost::edge_weight, g);
    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g, weights, parities);
    BOOST_TEST_EQ(w, 4);
    const bool parity0 = get(parities, 0);
    BOOST_TEST_EQ(parity0, get(parities, 1));
    BOOST_TEST_EQ(parity0, get(parities, 4));
    BOOST_TEST_EQ(parity0, get(parities, 5));
    const bool parity2 = get(parities, 2);
    BOOST_TEST_NE(parity0, parity2);
    BOOST_TEST_EQ(parity2, get(parities, 3));
    BOOST_TEST_EQ(parity2, get(parities, 6));
    BOOST_TEST_EQ(parity2, get(parities, 7));
}

void test1()
{
    { // if only one vertex, can't run stoer_wagner_min_cut
        undirected_graph g;
        add_vertex(g);
        BOOST_TEST_THROWS(
            boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map()),
            boost::bad_graph);
    }
    { // three vertices with one multi-edge
        edge_t edges[] = { { 0, 1 }, { 1, 2 }, { 1, 2 }, { 2, 0 } };
        weight_type ws[] = { 3, 1, 1, 1 };
        undirected_graph g(edges, edges + 4, ws, 3, 4);

        weight_map_type weights = get(boost::edge_weight, g);
        std::map< vertex_descriptor, bool > parity;
        boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
        const int w = boost::graph::stoer_wagner_min_cut(g, weights, parities);
        BOOST_TEST_EQ(w, 3);
        const bool parity2 = get(parities, 2), parity0 = get(parities, 0);
        BOOST_TEST_NE(parity2, parity0);
        BOOST_TEST_EQ(parity0, get(parities, 1));
    }
}

// a priority queue that is not empty on entry is rejected
void test_nonempty_queue_throws()
{
    undirected_graph g = mas_sw_oracle::make_weighted_graph(4, { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 } });
    weight_map_type weights = get(boost::edge_weight, g);
    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
    auto assignment_map = make_assignment_map(g);
    auto pq = make_maxheap(g);
    pq.push(0);
    BOOST_TEST_THROWS(
        boost::graph::stoer_wagner_min_cut(g, weights, parities, assignment_map, pq),
        std::invalid_argument);
}

// example by Daniel Trebbien
void test2()
{
    edge_t edges[] = { { 5, 2 }, { 0, 6 }, { 5, 6 }, { 3, 1 }, { 0, 1 },
        { 6, 3 }, { 4, 6 }, { 2, 4 }, { 5, 3 } };
    weight_type ws[] = { 1, 3, 4, 6, 4, 1, 2, 5, 2 };
    undirected_graph g(edges, edges + 9, ws, 7, 9);

    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g), parities);
    BOOST_TEST_EQ(w, 3);
    const bool parity2 = get(parities, 2);
    BOOST_TEST_EQ(parity2, get(parities, 4));
    const bool parity5 = get(parities, 5);
    BOOST_TEST_NE(parity2, parity5);
    BOOST_TEST_EQ(parity5, get(parities, 3));
    BOOST_TEST_EQ(parity5, get(parities, 6));
    BOOST_TEST_EQ(parity5, get(parities, 1));
    BOOST_TEST_EQ(parity5, get(parities, 0));
}

// example by Daniel Trebbien
void test3()
{
    edge_t edges[] = { 
        { 3, 4 }, { 3, 6 }, { 3, 5 }, { 0, 4 }, { 0, 1 },
        { 0, 6 }, { 0, 7 }, { 0, 5 }, { 0, 2 }, { 4, 1 }, { 1, 6 }, { 1, 5 },
        { 6, 7 }, { 7, 5 }, { 5, 2 }, { 3, 4 }
    };
    weight_type ws[] = { 0, 3, 1, 3, 1, 2, 6, 1, 8, 1, 1, 80, 2, 1, 1, 4 };
    undirected_graph g(edges, edges + 16, ws, 8, 16);

    weight_map_type weights = get(boost::edge_weight, g);
    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g, weights, parities);
    BOOST_TEST_EQ(w, 7);
    const bool parity1 = get(parities, 1);
    BOOST_TEST_EQ(parity1, get(parities, 5));
    const bool parity0 = get(parities, 0);
    BOOST_TEST_NE(parity1, parity0);
    BOOST_TEST_EQ(parity0, get(parities, 2));
    BOOST_TEST_EQ(parity0, get(parities, 3));
    BOOST_TEST_EQ(parity0, get(parities, 4));
    BOOST_TEST_EQ(parity0, get(parities, 6));
    BOOST_TEST_EQ(parity0, get(parities, 7));
}

// unweighted graph with a constant weight map
void test4()
{
    using vd = boost::graph_traits< undirected_unweighted_graph >::vertex_descriptor;
    using ed = boost::graph_traits< undirected_unweighted_graph >::edge_descriptor;

    edge_t edges[] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 0, 4 }, { 1, 4 },
        { 1, 5 }, { 2, 6 }, { 3, 6 }, { 3, 7 }, { 4, 5 }, { 5, 6 }, { 6, 7 },
        { 0, 4 }, { 6, 7 } };
    undirected_unweighted_graph g(edges, edges + 14, 8);

    std::map< vd, bool > parity;
    boost::associative_property_map< std::map< vd, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g,
        boost::make_constant_property< ed >(weight_type(1)), parities);
    BOOST_TEST_EQ(w, 2);
    const bool parity0 = get(parities, 0);
    BOOST_TEST_EQ(parity0, get(parities, 1));
    BOOST_TEST_EQ(parity0, get(parities, 4));
    BOOST_TEST_EQ(parity0, get(parities, 5));
    const bool parity2 = get(parities, 2);
    BOOST_TEST_NE(parity0, parity2);
    BOOST_TEST_EQ(parity2, get(parities, 3));
    BOOST_TEST_EQ(parity2, get(parities, 6));
    BOOST_TEST_EQ(parity2, get(parities, 7));
}

// Non regression test for github.com/boostorg/graph/issues/286
void test5()
{
    edge_t edges[] = { { 0, 1 }, { 0, 2 }, { 0, 3 }, { 1, 2 }, { 1, 3 },
        { 2, 3 }, { 4, 5 }, { 4, 6 }, { 4, 7 }, { 5, 6 }, { 5, 7 }, { 6, 7 },
        { 0, 4 } };
    weight_type ws[] = { 3, 3, 3, 2, 2, 2, 3, 3, 3, 2, 2, 2, 6 };
    undirected_graph g(edges, edges + 13, ws, 8, 13);

    weight_map_type weights = get(boost::edge_weight, g);
    std::map< vertex_descriptor, bool > parity;
    boost::associative_property_map< std::map< vertex_descriptor, bool > > parities(parity);
    const int w = boost::graph::stoer_wagner_min_cut(g, weights, parities);
    BOOST_TEST_EQ(w, 6);
    const bool parity0 = get(parities, 0);
    BOOST_TEST_EQ(parity0, get(parities, 1));
    BOOST_TEST_EQ(parity0, get(parities, 2));
    BOOST_TEST_EQ(parity0, get(parities, 3));
    const bool parity4 = get(parities, 4);
    BOOST_TEST_NE(parity0, parity4);
    BOOST_TEST_EQ(parity4, get(parities, 5));
    BOOST_TEST_EQ(parity4, get(parities, 6));
    BOOST_TEST_EQ(parity4, get(parities, 7));
}

// The input for the test_prgen family of tests comes from a program named
// prgen that comes with a package of min-cut solvers by Chandra Chekuri,
// Andrew Goldberg, David Karger, Matthew Levine, and Cliff Stein. prgen was
// used to generate input graphs and the solvers were used to verify the return
// value of stoer_wagner_min_cut on the input graphs.
//
// http://www.columbia.edu/~cs2035/code.html
//
// Only the cut weight is verified because prgen graphs often have several
// min-cuts.

// 3 min-cuts, via the fully positional overload with an explicit index map
void test_prgen_20_70_2()
{
    std::ifstream ifs((test_dir + "/prgen_input_graphs/prgen_20_70_2.net").c_str());
    undirected_graph g;
    boost::read_dimacs_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map(), ifs);

    std::map< vertex_descriptor, std::size_t > component;
    boost::associative_property_map< std::map< vertex_descriptor, std::size_t > > components(component);
    BOOST_TEST_EQ(boost::connected_components(g, components), 1U);

    auto assignment_map = make_assignment_map(g);
    auto pq = make_maxheap(g);
    const int w = boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g),
        boost::dummy_property_map(), assignment_map, pq, get(boost::vertex_index, g));
    BOOST_TEST_EQ(w, 3407);
}

// 7 min-cuts
void test_prgen_50_40_2()
{
    std::ifstream ifs((test_dir + "/prgen_input_graphs/prgen_50_40_2.net").c_str());
    undirected_graph g;
    boost::read_dimacs_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map(), ifs);

    std::map< vertex_descriptor, std::size_t > component;
    boost::associative_property_map< std::map< vertex_descriptor, std::size_t > > components(component);
    BOOST_TEST_EQ(boost::connected_components(g, components), 1U);

    const int w = boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map());
    BOOST_TEST_EQ(w, 10056);
}

// 6 min-cuts
void test_prgen_50_70_2()
{
    std::ifstream ifs((test_dir + "/prgen_input_graphs/prgen_50_70_2.net").c_str());
    undirected_graph g;
    boost::read_dimacs_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map(), ifs);

    std::map< vertex_descriptor, std::size_t > component;
    boost::associative_property_map< std::map< vertex_descriptor, std::size_t > > components(component);
    BOOST_TEST_EQ(boost::connected_components(g, components), 1U);

    const int w = boost::graph::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::dummy_property_map());
    BOOST_TEST_EQ(w, 21755);
}

int main(int argc, char* argv[])
{
    if (BOOST_TEST(argc == 2))
    {
        test_dir = argv[1];
        test0();
        test1();
        test_nonempty_queue_throws();
        test2();
        test3();
        test4();
        test5();
        test_prgen_20_70_2();
        test_prgen_50_40_2();
        test_prgen_50_70_2();
        test_curated_min_cuts();
    }
    return boost::report_errors();
}
