//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/reverse_graph.hpp>

int
main(int argc, char *argv[])
{
  if (argc < 3) {
    std::cerr << "usage: reachable-loop-tail.exe <in-file> <out-file>"
      << std::endl;
    return -1;
  }
  using namespace boost;
  GraphvizDigraph g_in;
  read_graphviz(argv[1], g_in);

  using Graph = adjacency_list < vecS, vecS, bidirectionalS,
    GraphvizVertexProperty,
    GraphvizEdgeProperty, GraphvizGraphProperty >;
  Graph g;
  copy_graph(g_in, g);

  graph_traits<GraphvizDigraph>::vertex_descriptor loop_tail = 6;
  using Color = color_traits<default_color_type>;
  default_color_type c;

  std::vector<default_color_type> reachable_to_tail(num_vertices(g));
  reverse_graph<Graph> reverse_g(g);
  depth_first_visit(reverse_g, loop_tail, default_dfs_visitor(),
                    make_iterator_property_map(reachable_to_tail.begin(),
                                               get(vertex_index, g), c));

  std::ofstream loops_out(argv[2]);
  loops_out << "digraph G {\n"
    << "  graph [ratio=\"fill\",size=\"3,3\"];\n"
    << "  node [shape=\"box\"];\n" << "  edge [style=\"bold\"];\n";

  auto vattr_map = get(vertex_attribute, g);
  for (const auto& vertex : make_range_pair(vertices(g_in))) {
    loops_out << vertex << "[label=\"" << vattr_map[vertex]["label"]
      << "\"";
    if (reachable_to_tail[vertex] != Color::white()) {
      loops_out << ", color=\"gray\", style=\"filled\"";
    }
    loops_out << "]\n";
  }

  for (const auto& edge : make_range_pair(edges(g_in)))
    loops_out << source(edge, g) << " -> " << target(edge, g) << ";\n";
  loops_out << "}\n";
  return EXIT_SUCCESS;
}
