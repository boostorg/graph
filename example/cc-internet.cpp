//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <fstream>
#include <vector>
#include <string>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>
#include "range_pair.hpp"

int
main()
{
  using namespace boost;
  GraphvizGraph g;
  read_graphviz("figs/cc-internet.dot", g);

  std::vector<int> component(num_vertices(g));

  connected_components
    (g, make_iterator_property_map(component.begin(),
                                   get(vertex_index, g), component[0]));

  auto vertex_attr_map = get(vertex_attribute, g);
  std::string color[] = {
  "white", "gray", "black", "lightgray"};
  for(const auto& vertex : make_range_pair(vertices(g))) {
    vertex_attr_map[vertex]["color"] = color[component[vertex]];
    vertex_attr_map[vertex]["style"] = "filled";
    if (vertex_attr_map[vertex]["color"] == "black")
      vertex_attr_map[vertex]["fontcolor"] = "white";
  }
  write_graphviz("figs/cc-internet-out.dot", g);

}
