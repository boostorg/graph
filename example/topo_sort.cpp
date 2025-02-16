//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <list>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <iterator>
#include <utility>

typedef std::pair< std::size_t, std::size_t > Pair;

/*
  Topological sort example

  The topological sort algorithm creates a linear ordering
  of the vertices such that if edge (u,v) appears in the graph,
  then u comes before v in the ordering.

  Sample output:

  A topological ordering: 2 5 0 1 4 3

*/

int main(int, char*[])
{
    // begin
    using namespace boost;

    /* Topological sort will need to color the graph.  Here we use an
       internal decorator, so we "property" the color to the graph.
       */
    typedef adjacency_list< vecS, vecS, directedS,
        property< vertex_color_t, default_color_type > >
        Graph;

    typedef boost::graph_traits< Graph >::vertex_descriptor Vertex;
    const auto edges = { Pair(0, 1), Pair(2, 4), Pair(2, 5), Pair(0, 3),
        Pair(1, 4), Pair(4, 3) };

    Graph G(std::begin(edges), std::end(edges), 6 /* vertices count */);

    auto id = get(vertex_index, G);

    typedef std::vector< Vertex > container;
    container c;
    topological_sort(G, std::back_inserter(c));

    std::cout << "A topological ordering: ";
    for (auto ii = c.rbegin(); ii != c.rend(); ++ii)
        std::cout << id[*ii] << " ";
    std::cout << std::endl;

    return 0;
}
