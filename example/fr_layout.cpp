// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/random_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topology.hpp>
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <boost/random/linear_congruential.hpp>
#include <boost/progress.hpp>
#include "range_pair.hpp"

using namespace boost;

void usage()
{
  std::cerr << "Usage: fr_layout [options] <width> <height>\n"
            << "Arguments:\n"
            << "\t<width>\tWidth of the display area (floating point)\n"
            << "\t<Height>\tHeight of the display area (floating point)\n\n"
            << "Options:\n"
            << "\t--iterations n\tNumber of iterations to execute.\n" 
            << "\t\t\tThe default value is 100.\n"
            << "Input:\n"
            << "  Input is read from standard input as a list of edges, one per line.\n"
            << "  Each edge contains two string labels (the endpoints) separated by a space.\n\n"
            << "Output:\n"
            << "  Vertices and their positions are written to standard output with the label,\n  x-position, and y-position of a vertex on each line, separated by spaces.\n";
}

using topology_type = boost::rectangle_topology<>;
using point_type = topology_type::point_type;

using Graph = adjacency_list<listS, vecS, undirectedS, 
                       property<vertex_name_t, std::string>>;

using Vertex = graph_traits<Graph>::vertex_descriptor;

using NameToVertex = std::map<std::string, Vertex>;

Vertex get_vertex(const std::string& name, Graph& g, NameToVertex& names)
{
  auto i = names.find(name);
  if (i == names.end())
    i = names.insert(std::make_pair(name, add_vertex(name, g))).first;
  return i->second;
}

class progress_cooling : public linear_cooling<double>
{
  using inherited = linear_cooling<double>;

 public:
  explicit progress_cooling(std::size_t iterations) : inherited(iterations) 
  {
    display.reset(new progress_display(iterations + 1, std::cerr));
  }

  double operator()()
  {
    ++(*display);
    return inherited::operator()();
  }

 private:
  std::shared_ptr<boost::progress_display> display;
};

int main(int argc, char* argv[])
{
  int iterations = 100;

  if (argc < 3) { usage(); return -1; }

  double width = 0;
  double height = 0;

  for (int arg_idx = 1; arg_idx < argc; ++arg_idx) {
    std::string arg = argv[arg_idx];
    if (arg == "--iterations") {
      ++arg_idx;
      if (arg_idx >= argc) { usage(); return -1; }
      iterations = std::stoi(argv[arg_idx]);
    } else {
      if (width == 0.0) width = std::stod(arg);
      else if (height == 0.0) height = std::stod(arg);
      else {
        usage();
        return -1;
      }
    }
  }

  if (width == 0.0 || height == 0.0) {
    usage();
    return -1;
  }

  Graph g;
  NameToVertex names;

  std::string source, target;
  while (std::cin >> source >> target) {
    add_edge(get_vertex(source, g, names), get_vertex(target, g, names), g);
  }
  
  using PositionVec = std::vector<point_type>;
  PositionVec position_vec(num_vertices(g));
  using PositionMap = iterator_property_map<PositionVec::iterator, 
                                property_map<Graph, vertex_index_t>::type>;
  PositionMap position(position_vec.begin(), get(vertex_index, g));

  minstd_rand gen;
  topology_type topo(gen, -width/2, -height/2, width/2, height/2);
  random_graph_layout(g, position, topo);
  fruchterman_reingold_force_directed_layout
    (g, position, topo,
     cooling(progress_cooling(iterations)));

  for (const auto& vertex : make_range_pair(vertices(g))) {
    std::cout << get(vertex_name, g, vertex) << '\t'
              << position[vertex][0] << '\t' << position[vertex][1] << std::endl;
  }
  return 0;
}
