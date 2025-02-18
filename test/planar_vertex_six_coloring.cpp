//=======================================================================
// Copyright 2024
// Authors: Hermann Stamm-Wilbrandt
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
// Demonstrates sequential_vertex_coloring() forced to take 15 colors
// on graph built with "create(15, g)", and planar_vertex_six_coloring()
// taking 6 colors only (both linear time algorithms).
//
#include <time.h>
#include <utility>
#include <iostream>
#include <fstream>
#include <boost/graph/sequential_vertex_coloring.hpp>
#include <boost/graph/planar_vertex_six_coloring.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace boost;

typedef adjacency_list< listS, vecS, undirectedS > Graph;
typedef graph_traits< Graph >::vertices_size_type vertices_size_type;
typedef graph_traits< Graph >::vertex_descriptor vertex_descriptor;
typedef graph_traits< Graph >::edge_descriptor edge_descriptor;


edge_descriptor create(Graph& g, int k);
void read_leda_graph(Graph& g, const char* gname);

// time measurements
clock_t start_;
#define __(blk) if (output) std::cerr << #blk << " "; start_ = clock(); blk \
    if (output) std::cerr << (clock()-start_)*1.0/CLOCKS_PER_SEC << std::endl;


int main(int argc, char*argv[])
{
    unsigned int k = (argc < 2) ? 15 : atoi(argv[1]);
    bool output = argc > 2;

    Graph g;
    edge_descriptor e;

    __(create(g, k);)

    BOOST_TEST(num_vertices(g) == 987);  // fib(k+1)
    BOOST_TEST(num_edges(g) == 1595);    // fib(k+2)-2

    std::vector< vertices_size_type > color_vec(num_vertices(g));
    auto color = make_container_vertex_map(color_vec, g);

    __(vertices_size_type num_colors = sequential_vertex_coloring(g, color);)

    BOOST_TEST(num_colors == k);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    __(vertices_size_type num_color6 = planar_vertex_six_coloring(g, color);)

    BOOST_TEST(num_color6 == 6);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    if (output)
    {
        std::cerr << num_vertices(g) << "/" << num_edges(g)
                  << " vertices/edges" << std::endl;
        std::cerr << num_colors << "/" << num_color6
                  << " colors used" << std::endl;
    }


    __(read_leda_graph(g, "planar_input_graphs/pentakis_dodecahedron.leda");)

    color_vec.resize(num_vertices(g));
    color = make_container_vertex_map(color_vec, g);

    BOOST_TEST(num_vertices(g) == 32);
    BOOST_TEST(num_edges(g) == 3*32-6);

    __(num_colors = sequential_vertex_coloring(g, color);)

    BOOST_TEST(num_colors == 5);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    __(num_color6 = planar_vertex_six_coloring(g, color);)

    BOOST_TEST(num_color6 == 4);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    if (output)
    {
        std::cerr << num_vertices(g) << "/" << num_edges(g)
                  << " vertices/edges" << std::endl;
        std::cerr << num_colors << "/" << num_color6
                  << " colors used" << std::endl;
    }


    __(read_leda_graph(g, "planar_input_graphs/maximal_planar_1000.leda");)

    color_vec.resize(num_vertices(g));
    color = make_container_vertex_map(color_vec, g);

    BOOST_TEST(num_vertices(g) == 1000);
    BOOST_TEST(num_edges(g) == 3*1000-6);

    __(num_colors = sequential_vertex_coloring(g, color);)

    BOOST_TEST(num_colors == 6);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    __(num_color6 = planar_vertex_six_coloring(g, color);)

    BOOST_TEST(num_color6 == 6);
    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    if (output)
    {
        std::cerr << num_vertices(g) << "/" << num_edges(g)
                  << " vertices/edges" << std::endl;
        std::cerr << num_colors << "/" << num_color6
                  << " colors used" << std::endl;
    }


    return boost::report_errors();
}


/*
                    creating C6 

                               -5-
                               /|\
...                               
                                        from C5
                                                 /
                                             ---4
                                            /  /|\ /
                     plus C3               /  / | 3
                              /           /  /  |/|\
       plus C1               2           /  /   2 | \
                /           / \ /       /  /   / \|  \
               0           0---1       0--1   0---1   0
*/
edge_descriptor create(Graph& g, int k)
{
    if (k < 4)
    {
        BOOST_ASSERT(k > 1);

        vertex_descriptor v = add_vertex(g);
        vertex_descriptor w = add_vertex(g);
        auto e = add_edge(v, w, g);
        if (k == 2)  return e.first;

        vertex_descriptor x = add_vertex(g);
        add_edge(v, x, g);
        return add_edge(w, x, g).first;
    }

    std::vector<edge_descriptor> es;
    while (--k > 1) { es.push_back(create(g, k--)); }

    vertex_descriptor v = add_vertex(g);
    if (k == 1)
    {
        vertex_descriptor w = add_vertex(g);
        add_edge(v, w, g);
        v = w;
    }

    edge_descriptor e;
    for (auto ei = es.rbegin(); ei != es.rend(); ++ei)
    {
        add_edge(source(*ei, g), v, g);
        e = add_edge(target(*ei, g), v, g).first;
    }
    return e;
}


void read_leda_graph(Graph& g, const char* gname)
{
    int n, m;
    std::string line;

    std::ifstream in(gname);
    BOOST_ASSERT(in);

    std::getline(in, line);
    in >> line >> line >> n;

    g = Graph(n);

    for (int i=1; i <= n; ++i) {
        in >> line;
    }

    in >> m;
    for (int i=1; i <= m; ++i) {
        int s, t, v;
        in >> s >> t >> v;
        add_edge(s-1, t-1, g);
    }
}
