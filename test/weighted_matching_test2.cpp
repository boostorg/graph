//=======================================================================
// Copyright (c) 2024 Joris van Rantwijk
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================

#include <boost/graph/maximum_weighted_matching.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/core/lightweight_test.hpp>
#include <iostream>
#include <limits>
#include <sstream>
#include <type_traits>
#include <vector>

using namespace boost;


template <typename WeightType>
using adj_vec_test_graph = adjacency_list<
    vecS,  // OutEdgeList
    vecS,  // VertexList
    undirectedS,  // DirectedS
    no_property,  // VertexProperty (vertex_index is implicit)
    property<edge_weight_t, WeightType>>;  // EdgeProperty

template <typename WeightType>
using adj_list_test_graph = adjacency_list<
    listS,  // OutEdgeList
    listS,  // VertexList
    undirectedS,  // DirectedS
    property<vertex_index_t, unsigned int>,  // VertexProperty
    property<edge_weight_t, WeightType>>;  // EdgeProperty

template <typename WeightType>
using adj_matrix_test_graph = adjacency_matrix<
    undirectedS,  // Directed
    no_property,  // VertexProperty (vertex_index is implicit)
    property<edge_weight_t, WeightType>>;  // EdgeProperty


template <typename WeightType>
struct edge_info
{
    unsigned int x, y;
    WeightType w;
};


// Initialize vertex_index if necessary.
template <typename Graph>
struct vertex_index_installer
{
    static void install(Graph&) {}
};

template <typename WeightType>
struct vertex_index_installer<adj_list_test_graph<WeightType>>
{
    static void install(adj_list_test_graph<WeightType>& g)
    {
        auto vrange = vertices(g);
        unsigned int i = 0;
        for (auto it = vrange.first; it != vrange.second; ++it, ++i)
            put(vertex_index, g, *it, i);
    }
};


template <typename Graph, typename WeightType>
Graph make_graph(
    unsigned int nv,
    const std::vector<edge_info<WeightType>>& edges_info)
{
    using EdgeProperty = property<edge_weight_t, WeightType>;
    Graph g(nv);
    vertex_index_installer<Graph>::install(g);
    for (const auto& e : edges_info)
        add_edge(vertex(e.x, g), vertex(e.y, g), EdgeProperty(e.w), g);
    return g;
}


template <typename Graph>
using graph_edge_weight_type = typename property_traits<
    typename property_map<Graph, edge_weight_t>::type>::value_type;


template <typename Graph, typename MateMap>
std::pair<graph_edge_weight_type<Graph>, bool>
check_matching(const Graph& g, const MateMap& mate)
{
    graph_edge_weight_type<Graph> matching_weight = 0;
    unsigned int n_matched_edges = 0;

    auto edge_range = edges(g);
    for (auto it = edge_range.first; it != edge_range.second; ++it)
    {
        if (mate[source(*it, g)] == target(*it, g))
        {
            ++n_matched_edges;
            matching_weight += get(edge_weight, g, *it);
        }
    }

    unsigned int n_matched_vertices = 0;
    auto vertex_range = vertices(g);
    for (auto it = vertex_range.first; it != vertex_range.second; ++it)
    {
        if (mate[*it] != graph_traits<Graph>::null_vertex())
        {
            if (mate[mate[*it]] != *it)
                return std::make_pair(0, false);
            ++n_matched_vertices;
        }
    }

    if (n_matched_vertices != 2 * n_matched_edges)
        return std::make_pair(0, false);

    return std::make_pair(matching_weight, true);
}


template <typename Graph>
std::string show_graph(const Graph& g)
{
    std::ostringstream ss;
    auto edge_range = edges(g);
    for (auto it = edge_range.first; it != edge_range.second; ++it)
    {
        unsigned int x = get(vertex_index, g, source(*it, g));
        unsigned int y = get(vertex_index, g, target(*it, g));
        auto w = get(edge_weight, g, *it);
        if (ss.tellp() > 0)
            ss << " ";
        ss << "{" << x << "," << y << "," << w << "}";
    }
    return ss.str();
}


template <typename Graph, typename WeightType>
void test_matching(const Graph& g, WeightType answer)
{
    using vertex_index_map_t =
        typename property_map<Graph, vertex_index_t>::type;
    using mate_t = vector_property_map<
        typename graph_traits<Graph>::vertex_descriptor, vertex_index_map_t>;

    mate_t mate(num_vertices(g));
    maximum_weighted_matching(g, mate);

    WeightType weight;
    bool consistent_matching;
    std::tie(weight, consistent_matching) = check_matching(g, mate);
    BOOST_TEST(consistent_matching);
    if (! consistent_matching)
    {
        std::cout << std::endl
                  << "Inconsistent answer for graph" << std::endl
                  << "  " << show_graph(g) << std::endl;
    }
    else
    {
        bool same_answer;
        if (std::numeric_limits<WeightType>::is_integer)
        {
            same_answer = (weight == answer);
        }
        else
        {
            WeightType max_error = std::max(1e-14, answer * 1e-14);
            same_answer = (std::abs(weight - answer) <= max_error);
        }

        BOOST_TEST(same_answer);
        if (! same_answer)
        {
            std::cout << std::endl
                      << "Wrong answer for graph" << std::endl
                      << "  " << show_graph(g) << std::endl
                      << "  found weight " << weight
                      << " while expecting " << answer << std::endl;
        }
    }
}


template <typename WeightType = long>
void run_test_graph(
    unsigned int nv,
    const std::vector<edge_info<WeightType>>& edges_info,
    WeightType answer)
{
    using vec_graph_t = adj_vec_test_graph<WeightType>;
    test_matching(
        make_graph<vec_graph_t, WeightType>(nv, edges_info), answer);

    using list_graph_t = adj_list_test_graph<WeightType>;
    test_matching(
        make_graph<list_graph_t, WeightType>(nv, edges_info), answer);

    using matrix_graph_t = adj_matrix_test_graph<WeightType>;
    test_matching(
        make_graph<matrix_graph_t, WeightType>(nv, edges_info), answer);
}


int main(int, char*[])
{
    // Simple test cases:
    run_test_graph(0, {}, 0);
    run_test_graph(1, {}, 0);
    run_test_graph(2, {}, 0);
    run_test_graph(2, {{0, 1, 1}}, 1);
    run_test_graph(3, {{0, 1, 10}, {1, 2, 11}}, 11);
    run_test_graph(4, {{0, 1, 5}, {1, 2, 11}, {2, 3, 5}}, 11);
    run_test_graph(4, {{0, 1, 5}, {1, 2, 11}, {2, 3, 7}}, 12);

    // Floating point edge weights:
    run_test_graph<double>(4, {
        {0, 1, 3.1415}, {1, 2, 2.7183}, {0, 2, 3.0}, {0, 3, 1.4142}},
        4.1325);

    // Negative edge weights:
    run_test_graph(4, {
        {0, 1, 2}, {0, 2, -2}, {1, 2, 1}, {1, 3, -1}, {2, 3, -6}},
        2);

    // Blossoms:
    run_test_graph(4, {{0, 1, 8}, {0, 2, 9}, {1, 2, 10}, {2, 3, 7}}, 15);

    run_test_graph(6, {
        {0, 1, 8}, {0, 2, 9}, {1, 2, 10}, {2, 3, 7}, {0, 5, 5}, {3, 4, 6}},
        21);

    run_test_graph(6, {
        {0, 1, 9}, {0, 2, 8}, {1, 2, 10}, {0, 3, 5}, {3, 4, 4}, {0, 5, 3}},
        17);

    run_test_graph(6, {
        {0, 1, 9}, {0, 2, 8}, {1, 2, 10}, {0, 3, 5}, {3, 4, 3}, {0, 5, 4}},
        17);

    run_test_graph(6, {
        {0, 1, 9}, {0, 2, 8}, {1, 2, 10}, {0, 3, 5}, {3, 4, 3}, {2, 5, 4}},
        16);

    run_test_graph(6, {
        {0, 1, 9}, {0, 2, 9}, {1, 2, 10}, {1, 3, 8}, {2, 4, 8},
        {3, 4, 10}, {4, 5, 6}},
        23);

    run_test_graph(8, {
        {0, 1, 10}, {0, 6, 10}, {1, 2, 12}, {2, 3, 20}, {2, 4, 20},
        {3, 4, 25}, {4, 5, 10}, {5, 6, 10}, {6, 7, 8}},
        48);

    run_test_graph(8, {
        {0, 1, 8}, {0, 2, 8}, {1, 2, 10}, {1, 3, 12}, {2, 4, 12},
        {3, 4, 14}, {3, 5, 12}, {4, 6, 12}, {5, 6, 14}, {6, 7, 12}},
        44);

    run_test_graph(8, {
        {0, 1, 23}, {0, 4, 22}, {0, 5, 15}, {1, 2, 25}, {2, 3, 22},
        {3, 4, 25}, {3, 7, 14}, {4, 6, 13}},
        67);

    run_test_graph(8, {
        {0, 1, 19}, {0, 2, 20}, {0, 7, 8}, {1, 2, 25}, {1, 3, 18},
        {2, 4, 18}, {3, 4, 13}, {3, 6, 7}, {4, 5, 7}},
        47);

    // Somewhat tricky test cases:
    run_test_graph(10, {
        {0, 1, 45}, {0, 4, 45}, {1, 2, 50}, {2, 3, 45}, {3, 4, 50},
        {0, 5, 30}, {2, 8, 35}, {3, 7, 35}, {4, 6, 26}, {8, 9, 5}},
        146);

    run_test_graph(10, {
        {0, 1, 45}, {0, 4, 45}, {1, 2, 50}, {2, 3, 45}, {3, 4, 50},
        {0, 5, 30}, {2, 8, 35}, {3, 7, 26}, {4, 6, 40}, {8, 9, 5}},
        151);

    run_test_graph(10, {
        {0, 1, 45}, {0, 4, 45}, {1, 2, 50}, {2, 3, 45}, {3, 4, 50},
        {0, 5, 30}, {2, 8, 35}, {3, 7, 28}, {4, 6, 26}, {8, 9, 5}},
        139);

    run_test_graph(12, {
        {0, 1, 45}, {0, 6, 45}, {1, 2, 50}, {2, 3, 45}, {3, 4, 95},
        {3, 5, 94}, {4, 5, 94}, {5, 6, 50}, {0, 7, 30}, {2, 10, 35},
        {4, 8, 36}, {6, 9, 26}, {10, 11, 5}},
        241);

    run_test_graph(10, {
        {0, 1, 40}, {0, 2, 40}, {1, 2, 60}, {1, 3, 55}, {2, 4, 55},
        {3, 4, 50}, {0, 7, 15}, {4, 6, 30}, {6, 5, 10}, {7, 9, 10},
        {3, 8, 30}},
        145);

    run_test_graph(6, {
        {0, 1, 2}, {0, 4, 3}, {1, 2, 7}, {1, 5, 2}, {2, 3, 9},
        {2, 5, 4}, {3, 4, 8}, {3, 5, 4}},
        15);

    run_test_graph(6, {
        {0, 1, 8}, {0, 2, 8}, {1, 2, 9}, {1, 3, 6}, {2, 4, 7},
        {3, 4, 8}, {3, 5, 5}},
        20);

    run_test_graph(7, {
        {0, 1, 7}, {0, 2, 7}, {1, 2, 9}, {0, 3, 7}, {0, 4, 7},
        {3, 4, 9}, {5, 6, 2}},
        20);

    run_test_graph(7, {
        {0, 1, 7}, {0, 4, 6}, {1, 2, 9}, {2, 3, 8}, {3, 4, 9},
        {3, 5, 1}, {4, 5, 1}},
        18);

    run_test_graph(5, {
        {0, 2, 7}, {0, 3, 3}, {1, 3, 1}, {2, 3, 5}, {2, 4, 5}},
        8);

    run_test_graph(7, {
        {0, 1, 15}, {0, 2, 10}, {0, 3, 11}, {0, 5, 17}, {1, 4, 12},
        {1, 5, 8}, {4, 6, 15}, {5, 6, 7}},
        34);

    run_test_graph(7, {
        {0, 1, 12}, {0, 3, 11}, {0, 4, 11}, {1, 3, 12}, {1, 5, 11},
        {2, 3, 14}, {2, 6, 14}, {3, 6, 11}},
        37);

    run_test_graph(7, {
        {0, 1, 19}, {0, 3, 17}, {0, 4, 19}, {1, 2, 15}, {1, 4, 21},
        {3, 5, 18}, {3, 6, 11}, {4, 5, 19}},
        52);

    run_test_graph(6, {
        {0, 1, 19}, {0, 2, 19}, {0, 3, 15}, {0, 4, 17}, {1, 2, 21},
        {3, 4, 16}, {4, 5, 10}},
        46);

    run_test_graph(6, {
        {0, 1, 48}, {0, 3, 42}, {0, 4, 57}, {1, 3, 51}, {1, 5, 36},
        {2, 3, 23}, {4, 5, 46}},
        117);

    run_test_graph(6, {
        {0, 2, 7}, {0, 5, 20}, {1, 2, 50}, {1, 4, 46}, {2, 3, 35},
        {2, 4, 8}, {2, 5, 25}, {3, 5, 47}},
        101);

    run_test_graph(5, {
        {0, 1, 48}, {0, 2, 44}, {0, 4, 48}, {1, 4, 36}, {3, 4, 31}},
        80);

    run_test_graph(6, {
        {3, 5, 71}, {2, 4, 36}, {3, 4, 27}, {5, 1, 29}, {2, 0, 48},
        {1, 0, 60}, {1, 3, 11}, {4, 5, 54}},
        167);

    run_test_graph(6, {
        {0, 2, 92}, {5, 4, 50}, {5, 0, 86}, {2, 1, 87}, {1, 3, 39},
        {4, 0, 2}, {4, 1, 83}, {0, 1, 56}},
        181);

    // Triangles, unit weight:
    run_test_graph(9, {
        {0, 1, 1}, {0, 2, 1}, {1, 2, 1}, {0, 3, 1},
        {3, 4, 1}, {3, 5, 1}, {4, 5, 1}, {4, 7, 1},
        {6, 7, 1}, {6, 8, 1}, {7, 8, 1}},
        4);

    // Trigger known bugs in a previous version of maximum_weighted_matching:

    // wrong answer
    run_test_graph(8, {
        {4, 7, 453}, {0, 4, 627}, {4, 6, 853}, {6, 7, 344}, {5, 6, 906},
        {4, 5, 689}, {2, 3, 741}, {2, 7, 746}, {3, 6, 647}, {1, 5, 385},
        {5, 7, 215}, {3, 7, 640}},
        2405);

    // wrong answer
    run_test_graph(12, {
        {0, 3, 448}, {0, 4, 919}, {0, 5, 918}, {0, 9, 72}, {1, 3, 830},
        {1, 4, 687}, {1, 5, 559}, {1, 6, 580}, {1, 7, 679}, {1, 8, 627},
        {2, 4, 585}, {2, 5, 835}, {2, 6, 822}, {2, 8, 462}, {2, 11, 380},
        {3, 4, 328}, {3, 5, 860}, {3, 7, 297}, {3, 8, 590}, {3, 11, 235},
        {4, 6, 692}, {4, 10, 227}, {4, 11, 354}, {5, 6, 160}, {5, 9, 400},
        {6, 8, 410}, {6, 9, 420}, {7, 11, 924}, {9, 11, 825}, {10, 11, 790}},
        4233);

    // assertion
    run_test_graph(9, {
        {4, 6, 796}, {3, 4, 553}, {0, 1, 792}, {1, 7, 1000}, {4, 5, 360},
        {5, 7, 183}, {4, 8, 694}, {0, 4, 741}, {0, 2, 483}, {5, 8, 228},
        {7, 8, 644}, {1, 3, 236}, {6, 7, 895}, {1, 6, 913}, {1, 8, 617}},
        2593);

    // assertion
    run_test_graph(10, {
        {0, 6, 892}, {2, 4, 517}, {4, 7, 560}, {1, 5, 828}, {1, 7, 831},
        {6, 7, 397}, {4, 5, 43}, {5, 6, 944}, {6, 9, 215}, {5, 7, 753},
        {4, 6, 901}, {1, 2, 530}, {2, 8, 384}, {3, 4, 499}, {5, 8, 190}},
        2674);

    // segmentation fault
    run_test_graph(7, {
        {5, 6, 799}, {0, 4, 601}, {0, 1, 578}, {0, 3, 373}, {4, 6, 675},
        {4, 5, 925}, {0, 5, 697}, {3, 4, 260}, {1, 3, 464}, {1, 5, 845},
        {0, 2, 176}, {2, 5, 685}},
        1938);

    // segmentation fault
    run_test_graph(8, {
        {2, 7, 420}, {6, 7, 414}, {2, 3, 421}, {2, 6, 854}, {1, 6, 997},
        {4, 5, 46}, {1, 2, 467}, {2, 4, 230}, {3, 7, 555}, {0, 3, 334},
        {0, 7, 341}, {0, 6, 634}},
        1805);

    // hang
    run_test_graph(7, {
        {2, 3, 837}, {1, 4, 458}, {3, 4, 291}, {5, 6, 601}, {1, 2, 202},
        {0, 4, 491}, {4, 5, 910}, {0, 1, 159}, {3, 5, 684}, {4, 6, 139},
        {0, 2, 792}, {1, 3, 232}},
        1934);

    // hang
    run_test_graph(12, {
        {0, 1, 856}, {0, 4, 462}, {0, 6, 874}, {0, 7, 406}, {0, 10, 294},
        {1, 3, 936}, {1, 5, 852}, {1, 7, 501}, {1, 8, 555}, {1, 10, 41},
        {2, 3, 325}, {2, 6, 748}, {2, 9, 808}, {3, 5, 870}, {3, 7, 25},
        {3, 8, 663}, {3, 9, 897}, {4, 7, 617}, {4, 9, 435}, {4, 10, 818},
        {4, 11, 933}, {5, 6, 608}, {5, 8, 636}, {6, 9, 274}, {6, 10, 279},
        {7, 9, 705}, {7, 10, 114}, {8, 11, 602}, {9, 10, 764}, {10, 11, 413}},
        4599);

    return boost::report_errors();
}
