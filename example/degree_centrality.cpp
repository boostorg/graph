// (C) Copyright Andrew Sutton 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)


//[degree_centrality_example
#include <iostream>
#include <iomanip>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/degree_centrality.hpp>

#include "helper.hpp"
#include "range_pair.hpp"

using namespace boost;

// The Actor type stores the name of each vertex in the graph.
struct Actor
{
    std::string name;
};

// Declare the graph type and its vertex and edge types.
using Graph = undirected_graph<Actor>;
using Vertex = graph_traits<Graph>::vertex_descriptor;
using Edge = graph_traits<Graph>::edge_descriptor;

// The name map provides an abstract accessor for the names of
// each vertex. This is used during graph creation.
using NameMap = property_map<Graph, std::string Actor::*>::type;

// Declare a container type for degree centralities and its
// corresponding property map.
using CentralityProperty = exterior_vertex_property<Graph, unsigned>;
using CentralityContainer = CentralityProperty::container_type;
using CentralityMap = CentralityProperty::map_type;

int
main(int argc, char *argv[])
{
    // Create the graph and a property map that provides access
    // to the actor names.
    Graph g;
    NameMap nm(get(&Actor::name, g));

    // Read the graph from standard input.
    read_graph(g, nm, std::cin);

    // Compute the degree centrality for graph.
    CentralityContainer cents(num_vertices(g));
    CentralityMap cm(cents, g);
    all_degree_centralities(g, cm);

    // Print the degree centrality of each vertex.
    for(const auto& vertex : make_range_pair(vertices(g))) {
      std::cout << std::setiosflags(std::ios::left) << std::setw(12)
             << g[vertex].name << cm[vertex] << std::endl;
    }

    return 0;
}
//]
