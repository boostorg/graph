//=======================================================================
// Copyright 2013 Maciej Piechotka
// Authors: Maciej Piechotka
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <boost/config.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_coloring.hpp>
#include <boost/graph/properties.hpp>

/*
  Sample output
  Colored using 5 colors
    a-d: 4
    a-f: 0
    b-c: 2
    b-e: 3
    b-g: 1
    b-j: 0
    c-d: 0
    c-e: 1
    d-f: 2
    d-i: 1
    e-g: 4
    f-g: 3
    f-h: 1
    g-h: 0
*/

int main(int, char *[])
{
  using namespace boost;
  using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, size_t, no_property>;

  using Pair = std::pair<std::size_t, std::size_t>;
  Pair edges[] = { Pair(0,3), //a-d
                     Pair(0,5),  //a-f
                     Pair(1,2),  //b-c
                     Pair(1,4),  //b-e
                     Pair(1,6),  //b-g
                     Pair(1,9),  //b-j
                     Pair(2,3),  //c-d
                     Pair(2,4),  //c-e
                     Pair(3,5),  //d-f
                     Pair(3,8),  //d-i
                     Pair(4,6),  //e-g
                     Pair(5,6),  //f-g
                     Pair(5,7),  //f-h
                     Pair(6,7) }; //g-h

  Graph G(10);

  for (const auto& e : edges)
    add_edge(e.first, e.second, G);

  auto colors = edge_coloring(G, get(edge_bundle, G));

  std::cout << "Colored using " << colors << " colors" << std::endl;
  for (const auto& e : edges) {
    std::cout << "  " << (char)('a' + e.first) << "-" << (char)('a' + e.second) << ": " << G[edge(e.first, e.second, G).first] << std::endl;
  }

  return 0;
}

