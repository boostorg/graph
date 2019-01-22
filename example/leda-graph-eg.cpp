//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/graph/leda_graph.hpp>
#include <iostream>
#undef string                   // LEDA macro!
int
main()
{
  using namespace boost;
  using graph_t = leda::GRAPH<std::string, int>;
  graph_t g;
  g.new_node("Philoctetes");
  g.new_node("Heracles");
  g.new_node("Alcmena");
  g.new_node("Eurystheus");
  g.new_node("Amphitryon");
  using NodeMap = property_map<graph_t, vertex_all_t>::type;
  auto node_name_map = get(vertex_all, g);
  for (const auto& vertex : make_range_pair(vertices(g)))
    std::cout << node_name_map[vertex] << std::endl;
  return EXIT_SUCCESS;
}
