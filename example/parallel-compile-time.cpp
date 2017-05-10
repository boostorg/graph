//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <fstream>
#include <iostream>
#include <numeric>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/property_iter_range.hpp>
#include <boost/graph/depth_first_search.hpp>   // for default_dfs_visitor
#include "range_pair.hpp"

namespace std
{
  template <typename T>
    std::istream & operator >> (std::istream & in, std::pair<T, T> &p)
  {
    in >> p.first >> p.second;
    return in;
  }
}

namespace boost
{
  enum vertex_compile_cost_t { vertex_compile_cost };
  BOOST_INSTALL_PROPERTY(vertex, compile_cost);
}

using namespace boost;

using file_dep_graph2 = adjacency_list<listS, // Store out-edges of each vertex in a std::list
  listS,                        // Store vertex set in a std::list
  directedS,                    // The file dependency graph is directed
  // vertex properties
  property < vertex_name_t, std::string, 
  property < vertex_compile_cost_t, float,
  property < vertex_distance_t, float, 
  property<vertex_color_t, default_color_type>>>>,
  // an edge property
  property<edge_weight_t, float >>;

using vertex_t = graph_traits<file_dep_graph2>::vertex_descriptor;
using edge_t = graph_traits<file_dep_graph2>::edge_descriptor;


template <typename Graph, typename ColorMap, typename Visitor> void
dfs_v2(const Graph & g,
       typename graph_traits<Graph>::vertex_descriptor u,
       ColorMap color, Visitor vis)
{
  using color_type = typename property_traits<ColorMap>::value_type;
  using ColorT = color_traits<color_type>;
  color[u] = ColorT::gray();
  vis.discover_vertex(u, g);
  for (const auto& edge : make_range_pair(out_edges(u, g)))
    if (color[target(edge, g)] == ColorT::white()) {
      vis.tree_edge(edge, g);
      dfs_v2(g, target(edge, g), color, vis);
    } else if (color[target(edge, g)] == ColorT::gray())
      vis.back_edge(edge, g);
    else
      vis.forward_or_cross_edge(edge, g);
  color[u] = ColorT::black();
  vis.finish_vertex(u, g);
}

template <typename Graph, typename Visitor, typename ColorMap> void
generic_dfs_v2(const Graph & g, Visitor vis, ColorMap color)
{
  using ColorValue = typename property_traits <ColorMap >::value_type;
  using ColorT = color_traits<ColorValue> ;
  for (const auto& vertex : make_range_pair(vertices(g)))
    color[vertex] = ColorT::white();
  for (const auto& vertex : make_range_pair(vertices(g)))
    if (color[vertex] == ColorT::white())
      dfs_v2(g, vertex, color, vis);
}


template <typename OutputIterator> 
struct topo_visitor: public default_dfs_visitor
{
  topo_visitor(OutputIterator & order):
  topo_order(order)
  {
  }
  template <typename Graph> void
  finish_vertex(typename graph_traits<Graph>::vertex_descriptor u,
                const Graph &)
  {
    *topo_order++ = u;
  }
  OutputIterator & topo_order;
};

template <typename Graph, typename OutputIterator, typename ColorMap> void
topo_sort(const Graph & g, OutputIterator topo_order, ColorMap color)
{
  topo_visitor<OutputIterator> vis(topo_order);
  generic_dfs_v2(g, vis, color);
}


using name_map_t = property_map<file_dep_graph2, vertex_name_t>::type;
using compile_cost_map_t = property_map<file_dep_graph2, vertex_compile_cost_t>::type;
using distance_map_t = property_map<file_dep_graph2, vertex_distance_t>::type;
using color_map_t = property_map<file_dep_graph2, vertex_color_t>::type;


int
main()
{
  std::ifstream file_in("makefile-dependencies.dat");
  using size_type = graph_traits<file_dep_graph2>::vertices_size_type;
  size_type n_vertices;
  file_in >> n_vertices;        // read in number of vertices
  std::istream_iterator < std::pair < size_type,
    size_type >> input_begin(file_in), input_end;
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  // VC++ can't handle the iterator constructor
  file_dep_graph2 g;
  using vertex_t = graph_traits<file_dep_graph2 >::vertex_descriptor;
  std::vector<vertex_t> id2vertex;
  for (std::size_t v = 0; v < n_vertices; ++v)
    id2vertex.emplace_back(add_vertex(g));
  while (input_begin != input_end) {
    auto [i, j] = *input_begin++;
    add_edge(id2vertex[i], id2vertex[j], g);
  }
#else
  file_dep_graph2 g(input_begin, input_end, n_vertices);
#endif

  auto name_map =
    get(vertex_name, g);
  auto compile_cost_map =
    get(vertex_compile_cost, g);
  auto distance_map =
    get(vertex_distance, g);
  auto color_map =
    get(vertex_color, g);

  {
    std::ifstream name_in("makefile-target-names.dat");
    std::ifstream compile_cost_in("target-compile-costs.dat");
    for (const auto& vertex : make_range_pair(vertices(g))) {
      name_in >> name_map[vertex];
      compile_cost_in >> compile_cost_map[vertex];
    }

  }
  std::vector<vertex_t> topo_order(num_vertices(g));
  topo_sort(g, topo_order.rbegin(), color_map);

  // find source vertices with zero in-degree by marking all vertices with incoming edges
  for (const auto& vertex : make_range_pair(vertices(g)))
    color_map[vertex] = white_color;
  for (const auto& vertex : make_range_pair(vertices(g)))
    for (const auto& vertex_adjacent : make_range_pair(adjacent_vertices(vertex, g)))
      color_map[vertex_adjacent] = black_color;

  // initialize distances to zero, or for source vertices, to the compile cost
  for (const auto& vertex : make_range_pair(vertices(g)))
    if (color_map[vertex] == white_color)
      distance_map[vertex] = compile_cost_map[vertex];
    else
      distance_map[vertex] = 0;


  for (const auto& u : topo_order) {
    for (const auto& vertex : make_range_pair(adjacent_vertices(u, g)))
      if (distance_map[vertex] < distance_map[u] + compile_cost_map[vertex])
        distance_map[vertex] = distance_map[u] + compile_cost_map[vertex];
  }

  auto [ci, ci_end] = get_property_iter_range(g, vertex_distance);
  std::cout << "total (parallel) compile time: "
    << *std::max_element(ci, ci_end) << std::endl;

  return 0;
}
