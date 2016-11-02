//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <boost/graph/edmonds_karp_max_flow.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/graph_utility.hpp>
#include "range_pair.hpp"

// Use a DIMACS network flow file as stdin.
// edmonds-karp-eg < max_flow.dat
//
// Sample output:
//  c  The total flow:
//  s 13
//
//  c flow values:
//  f 0 6 3
//  f 0 1 6
//  f 0 2 4
//  f 1 5 1
//  f 1 0 0
//  f 1 3 5
//  f 2 4 4
//  f 2 3 0
//  f 2 0 0
//  f 3 7 5
//  f 3 2 0
//  f 3 1 0
//  f 4 5 4
//  f 4 6 0
//  f 5 4 0
//  f 5 7 5
//  f 6 7 3
//  f 6 4 0
//  f 7 6 0
//  f 7 5 0

int
main()
{
  using namespace boost;

  using Traits = adjacency_list_traits<vecS, vecS, directedS>;
  using Graph = adjacency_list < listS, vecS, directedS,
    property<vertex_name_t, std::string>,
    property < edge_capacity_t, long,
    property < edge_residual_capacity_t, long,
    property<edge_reverse_t, Traits::edge_descriptor>>>>;

  Graph g;

  auto capacity = get(edge_capacity, g);
  auto rev = get(edge_reverse, g);
  auto residual_capacity = get(edge_residual_capacity, g);

  Traits::vertex_descriptor s, t;
  read_dimacs_max_flow(g, capacity, rev, s, t);

#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  std::vector<default_color_type> color(num_vertices(g));
  std::vector<Traits::edge_descriptor> pred(num_vertices(g));
  long flow = edmonds_karp_max_flow
    (g, s, t, capacity, residual_capacity, rev, &color[0], &pred[0]);
#else
  long flow = edmonds_karp_max_flow(g, s, t);
#endif

  std::cout << "c  The total flow:" << std::endl;
  std::cout << "s " << flow << std::endl << std::endl;

  std::cout << "c flow values:" << std::endl;
  for(const auto& vertex : make_range_pair(vertices(g)))
    for (const auto& edge : make_range_pair(out_edges(vertex, g)))
      if (capacity[edge] > 0)
        std::cout << "f " << vertex << " " << target(edge, g) << " "
          << (capacity[edge] - residual_capacity[edge]) << std::endl;

  return EXIT_SUCCESS;
}
