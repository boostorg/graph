//=======================================================================
// Copyright 2024
// Author: Hermann Stamm-Wilbrandt
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/planar_vertex_six_coloring.hpp>

using namespace boost;

// => ../test/planar_vertex_six_coloring.cpp for application to other planar graphs
//
template< class graph >  void simple_maximal_planar_random_graph(graph& g, int n);


// time measurements
clock_t start_;
#define _(blk) std::cerr << #blk << " "; start_ = clock(); blk \
    std::cerr << (clock()-start_)*1.0/CLOCKS_PER_SEC << "s" << std::endl;


int main(int argc, char *argv[]) {
    typedef adjacency_list< listS, vecS, undirectedS > Graph;
    typedef graph_traits< Graph >::vertices_size_type vertices_size_type;
    Graph g;

    _(simple_maximal_planar_random_graph(g, argc < 2 ? 333335 : atoi(argv[1]));)

    std::vector< vertices_size_type > color_vec(num_vertices(g));
    auto color = make_container_vertex_map(color_vec, g);

    std::cout << num_vertices(g) << " vertices" << std::endl;
    std::cout << num_edges(g) << " egdes" << std::endl;

    _(vertices_size_type num_colors = planar_vertex_six_coloring(g, color);)
    std::cout << num_colors << " colors" << std::endl;

    BGL_FORALL_EDGES(e, g, Graph)
        { BOOST_ASSERT(get(color, source(e, g)) != get(color, target(e, g))); }

    std::cout << "vertex coloring verified" << std::endl;

    return 0;
}


/*
https://www.researchgate.net/publication/360912158_A_simple_linear_time_algorithm_for_embedding_maximal_planar_graphs_1

"simple_maximal_planar_random_graph()" implements only type==3 of chapter 8.
With type==4 and type==5 added it would be full blown "maximal_planar_random_graph()".


     triangular face T[t] transformation, t chosen randomly:

            T[t][0]                    T[t][0]
         +--#--+                    +--#--+    
         |     |                    |  |  |
         |     |                    |  |  |
         |     |        ==>         |  i  |
         |     |                    | / \ |
         |     |                    |/   \|
         #-----#                    #-----#
   T[t][1]     T[t][2]        T[t][1]     T[t][2]
*/
template< class graph >  void simple_maximal_planar_random_graph(graph& g, int n)
{
    BOOST_ASSERT(n > 2);
    g.clear();
    std::vector< typename graph::vertex_descriptor> V;  V.reserve(n);
    V[0] = add_vertex(g); V[1] = add_vertex(g); V[2] = add_vertex(g);
    add_edge(V[0], V[1], g); add_edge(V[0], V[2], g); add_edge(V[1], V[2], g);
    std::vector<std::vector<unsigned>> T;
    // start with inside and outside face of complete graph on 3 vertices
    T.reserve(2*n-4); T.push_back({0, 1, 2}); T.push_back({0, 1, 2});
    for (int i=3; i < n; ++i)
    {
        unsigned t = random() % T.size();
        V[i] = add_vertex(g);
        unsigned v;
        BOOST_FOREACH(v, T[t])  { add_edge(V[v], V[i], g); }
        T.push_back(T[t]); T.back()[0] = i;
        T.push_back(T[t]); T.back()[1] = i;
        T[t][2] = i;
    }
}
