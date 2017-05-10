//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>
#include <string>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_iter_range.hpp>
#include "range_pair.hpp"

int
main()
{
  using namespace boost;
  using graph_t = adjacency_list<listS, vecS, directedS,
    property<vertex_name_t, std::string>>;
  graph_t g(3);

  const char *vertex_names[] = { "Kubrick", "Clark", "Hal" };
  int i = 0;
  for (auto& vertex : make_range_pair(get_property_iter_range(g, vertex_name))) {
    vertex = vertex_names[i];
    ++i;
  }

  auto [v, v_end] = get_property_iter_range(g, vertex_name);
  std::copy(v, v_end, std::ostream_iterator<std::string> (std::cout, " "));
  std::cout << std::endl;
  return 0;
}
