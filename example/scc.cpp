//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <fstream>
#include <map>
#include <string>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/graphviz.hpp>

int
main()
{
  using namespace boost;
  GraphvizDigraph g;
  read_graphviz("figs/scc.dot", g);

  using vertex_t = graph_traits<GraphvizDigraph>::vertex_descriptor;
  std::map<vertex_t, int> component;

  strong_components(g, make_assoc_property_map(component));

  auto vertex_attr_map = get(vertex_attribute, g);
  std::string color[] = {
  "white", "gray", "black", "lightgray"};
  for (const auto& vertex : make_range_pair(vertices(g))) {
    vertex_attr_map[vertex]["color"] = color[component[vertex]];
    vertex_attr_map[vertex]["style"] = "filled";
    if (vertex_attr_map[vertex]["color"] == "black")
      vertex_attr_map[vertex]["fontcolor"] = "white";
  }
  write_graphviz("figs/scc-out.dot", g);

  return EXIT_SUCCESS;
}
