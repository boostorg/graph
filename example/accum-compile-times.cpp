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
#include <iterator>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_iter_range.hpp>

namespace std
{
  template <typename T>
  std::istream& operator >> (std::istream& in, std::pair<T, T> &p)
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
  listS,                       // Store vertex set in a std::list
  directedS,                   // The file dependency graph is directed
  // vertex properties
  property<vertex_name_t, std::string,
  property<vertex_compile_cost_t, float,
  property<vertex_distance_t, float,
  property<vertex_color_t, default_color_type>>>>,
  // an edge property
  property<edge_weight_t, float>>;

using vertex_t = graph_traits<file_dep_graph2>::vertex_descriptor;
using edge_t = graph_traits<file_dep_graph2>::edge_descriptor;

int
main()
{
  std::ifstream file_in("makefile-dependencies.dat");
  using size_type = graph_traits<file_dep_graph2>::vertices_size_type;
  size_type n_vertices;
  file_in >> n_vertices;        // read in number of vertices
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  // std::istream_iterator causes trouble with VC++
  std::vector<vertex_t> id2vertex;
  file_dep_graph2 g;
  for (std::size_t i = 0; i < n_vertices; ++i)
    id2vertex.emplace_back(add_vertex(g));
  std::pair<size_type, size_type> p;
  while (file_in >> p) 
    add_edge(id2vertex[p.first], id2vertex[p.second], g);
#else
  std::istream_iterator<std::pair<size_type, size_type>>
    input_begin(file_in), input_end;
  file_dep_graph2 g(input_begin, input_end, n_vertices);
#endif

  auto name_map = get(vertex_name, g);
  auto compile_cost_map = get(vertex_compile_cost, g);

  std::ifstream name_in("makefile-target-names.dat");
  std::ifstream compile_cost_in("target-compile-costs.dat");
  graph_traits<file_dep_graph2>::vertex_iterator vi, vi_end;
  for (std::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    name_in >> name_map[*vi];
    compile_cost_in >> compile_cost_map[*vi];
  }

  graph_property_iter_range < file_dep_graph2,
    vertex_compile_cost_t >::iterator ci, ci_end;
  std::tie(ci, ci_end) = get_property_iter_range(g, vertex_compile_cost);
  std::cout << "total (sequential) compile time: "
    << std::accumulate(ci, ci_end, 0.0) << std::endl;

  return 0;
}

