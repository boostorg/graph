//=======================================================================
// Copyright 2024
// Author: Hermann Stamm-Wilbrandt
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <iostream>
#include <boost/foreach.hpp>
#include <boost/graph/undirected_graph_constant_time_edge_add_and_remove.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/graph_utility.hpp>

typedef boost::undirected_graph_constant_time_edge_add_and_remove<
    boost::no_property, boost::no_property, boost::no_property > ugraph_o1;

typedef boost::undirected_graph<
    boost::no_property, boost::no_property, boost::no_property > ugraph;


/*
    Wheel graphs      W5   4      W7  6---1
                          /|\        / \ / \
                         / | \      5---0---2
                        3--0--1      \ / \ /
                         \ | /        4---3
                          \|/
                           2
*/
template< class Graph >  void wheel(Graph& g, int n);


/*
   Demo function removing the edges for all but central vertex of
   wheel graph Wn, showing roughly linear time for ugraph_01 
   [because of O(1) remove_edge()] and quadratic time for ugraph.

   Very simplified version of what planar_vertex_six_coloring() does.
   Proof that boost::undirected_graph_constant_time_edge_add_and_remove
   class is essential for linear runtime of that vertex coloring algorithm.
*/
template< class Graph >  float clear_vertices(Graph& g, int n)
{
    wheel(g, n);

    std::vector< typename Graph::vertex_descriptor > vec;
    vec.reserve(num_vertices(g));

    BGL_FORALL_VERTICES_T(v, g, Graph)  { vec.push_back(v); }

    clock_t start = clock();

    typename Graph::vertex_descriptor u;
    BOOST_REVERSE_FOREACH(u, vec)  { g.clear_vertex(u); }

    return (clock()-start)*1.0/CLOCKS_PER_SEC;
}


int main(int argc, char*argv[]) {
    int n = argc < 2 ? 2000 : atoi(argv[1]);
    ugraph_o1 UO1;
    ugraph U;

    std::cout << "Wn    ugraph_o1 ugraph     runtimes [s]" << std::endl;

    for (int i=0; i <= 4; ++i, n*=2)
    {
        std::cout << n << "  "
                  << clear_vertices(UO1, n) << "  "
                  << clear_vertices(U, n) << std::endl;
    }

    return 0;
}


template< class Graph >
void wheel(Graph& g, int n)
{
    typedef typename Graph::vertex_descriptor vertex_descriptor;

    BOOST_ASSERT(n > 3);

    g = Graph();

    vertex_descriptor v = add_vertex(g);
    vertex_descriptor w = add_vertex(g);
    g.add_edge(v, w);

    vertex_descriptor x = w;

    for (int i=3; i <= n; ++i)
    {
        vertex_descriptor y = add_vertex(g);
        g.add_edge(v, y);
        g.add_edge(x, y);
        x = y;
    }

    g.add_edge(x, w);
}
