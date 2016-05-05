//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <deque>                // to store the vertex ordering
#include <vector>
#include <list>
#include <iostream>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/topological_sort.hpp>

int
main()
{
  using namespace boost;
  const char *tasks[] = {
    "pick up kids from school",
    "buy groceries (and snacks)",
    "get cash at ATM",
    "drop off kids at soccer practice",
    "cook dinner",
    "pick up kids from soccer",
    "eat dinner"
  };

  std::vector<std::list<int>> g = {
    {3},
    {3, 4},
    {1},
    {5},
    {6},
    {6},
    {}};

  std::deque<int> topo_order;

  topological_sort(g, std::front_inserter(topo_order),
                   vertex_index_map(identity_property_map()));

  for (const auto& vertex : topo_order)
    std::cout << tasks[vertex] << std::endl;

  return EXIT_SUCCESS;
}
