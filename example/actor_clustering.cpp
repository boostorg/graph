// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine

// This program performs betweenness centrality (BC) clustering on the
// actor collaboration graph available at
// http://www.nd.edu/~networks/resources/actor/actor.dat.gz and outputs the
// result of clustering in Pajek format.
//
// This program mimics the BC clustering algorithm program implemented
// by Shashikant Penumarthy for JUNG, so that we may compare results
// and timings.
#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/tokenizer.hpp>
#include <map>
#include "range_pair.hpp"

using namespace boost;

struct Actor
{
  Actor(int id = -1) : id(id) {}

  int id;
};

using ActorGraph = adjacency_list<vecS, vecS, undirectedS, Actor,
  property<edge_centrality_t, double>>;
using Vertex = graph_traits<ActorGraph>::vertex_descriptor;
using Edge = graph_traits<ActorGraph>::edge_descriptor;

void load_actor_graph(std::istream& in, ActorGraph& g)
{
  std::map<int, Vertex> actors;

  std::string line;
  while (getline(in, line)) {
    std::vector<Vertex> actors_in_movie;

    // Map from the actor numbers on this line to the actor vertices
    using Tok = tokenizer<char_separator<char>>;
    Tok tok(line, char_separator<char>(" "));
    for (const auto& id : tok) {
      auto actor_id = std::stoi(id);
      auto v = actors.find(actor_id);
      if (v == actors.end()) {
        auto new_vertex = add_vertex(Actor(actor_id), g);
        actors[actor_id] = new_vertex;
        actors_in_movie.emplace_back(new_vertex);
      } else {
        actors_in_movie.emplace_back(v->second);
      }
    }

    for (auto i = actors_in_movie.begin();
         i != actors_in_movie.end(); ++i) {
      for (auto j = i + 1;
           j != actors_in_movie.end(); ++j) {
        if (!edge(*i, *j, g).second) add_edge(*i, *j, g);
      }
    }
  }
}

template<typename Graph, typename VertexIndexMap, typename VertexNameMap>
std::ostream& 
write_pajek_graph(std::ostream& out, const Graph& g, 
                  VertexIndexMap vertex_index, VertexNameMap vertex_name)
{
  out << "*Vertices " << num_vertices(g) << '\n';
  for (const auto& vertex : make_range_pair(vertices(g))) {
    out << get(vertex_index, vertex)+1 << " \"" << get(vertex_name, vertex) << "\"\n";
  }

  out << "*Edges\n";
  for (const auto& edge : make_range_pair(edges(g))) {
    out << get(vertex_index, source(edge, g))+1 << ' ' 
        << get(vertex_index, target(edge, g))+1 << " 1.0\n"; // HACK!
  }
  return out;
}

class actor_clustering_threshold : public bc_clustering_threshold<double>
{
  using inherited = bc_clustering_threshold<double>;

 public:
  actor_clustering_threshold(double threshold, const ActorGraph& g,
                             bool normalize)
    : inherited(threshold, g, normalize), iter(1) { }

  bool operator()(double max_centrality, Edge e, const ActorGraph& g)
  {
    std::cout << "Iter: " << iter << " Max Centrality: " 
              << (max_centrality / dividend) << std::endl;
    ++iter;
    return inherited::operator()(max_centrality, e, g);
  }

 private:
  unsigned int iter;
};

int main(int argc, char* argv[])
{
  std::string in_file;
  std::string out_file;
  double threshold = -1.0;
  bool normalize = false;

  // Parse command-line options
  {
    int on_arg = 1;
    while (on_arg < argc) {
      std::string arg(argv[on_arg]);
      if (arg == "-in") {
        ++on_arg; assert(on_arg < argc);
        in_file = argv[on_arg];
      } else if (arg == "-out") {
        ++on_arg; assert(on_arg < argc);
        out_file = argv[on_arg];
      } else if (arg == "-threshold") {
        ++on_arg; assert(on_arg < argc);
        threshold = std::stod(argv[on_arg]);
      } else if (arg == "-normalize") {
        normalize = true;
      } else {
        std::cerr << "Unrecognized parameter \"" << arg << "\".\n";
        return -1;
      }
      ++on_arg;
    }

    if (in_file.empty() || out_file.empty() || threshold < 0) {
      std::cerr << "error: syntax is actor_clustering [options]\n\n"
                << "options are:\n"
                << "\t-in <infile>\tInput file\n"
                << "\t-out <outfile>\tOutput file\n"
                << "\t-threshold <value>\tA threshold value\n"
                << "\t-normalize\tNormalize edge centrality scores\n";
      return -1;
    }
  }

  ActorGraph g;

  // Load the actor graph
  {
    std::cout << "Building graph." << std::endl;
    std::ifstream in(in_file.c_str());
    if (!in) {
      std::cerr << "Unable to open file \"" << in_file << "\" for input.\n";
      return -2;
    }
    load_actor_graph(in, g);
  }

  // Run the algorithm
  std::cout << "Clusting..." << std::endl;
  betweenness_centrality_clustering(g, 
    actor_clustering_threshold(threshold, g, normalize), 
    get(edge_centrality, g));

  // Output the graph
  {
    std::cout << "Writing graph to file: " << out_file << std::endl;
    std::ofstream out(out_file.c_str());
    if (!out) {
      std::cerr << "Unable to open file \"" << out_file << "\" for output.\n";
      return -3;
    }
    write_pajek_graph(out, g, get(vertex_index, g), get(&Actor::id, g));
  }
  return 0;
}
