// (C) Copyright Andrew Sutton 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)


//[code_clustering_coefficient
#include <iostream>
#include <iomanip>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/clustering_coefficient.hpp>
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

// The clustering property, container, and map define the containment
// and abstract accessor for the clustering coefficients of vertices.
using ClusteringProperty = exterior_vertex_property<Graph, float>;
using ClusteringContainer = ClusteringProperty::container_type;
using ClusteringMap = ClusteringProperty::map_type;

int
main(int argc, char *argv[])
{
    // Create the graph and a name map that provides access to
    // then actor names.
    Graph g;
    NameMap nm(get(&Actor::name, g));

    // Read the graph from standard input.
    read_graph(g, nm, std::cin);

    // Compute the clustering coefficients of each vertex in the graph
    // and the mean clustering coefficient which is returned from the
    // computation.
    ClusteringContainer coefs(num_vertices(g));
    ClusteringMap cm(coefs, g);
    auto cc = all_clustering_coefficients(g, cm);

    // Print the clustering coefficient of each vertex.
    for(const auto& vertex : make_range_pair(vertices(g))) {
      std::cout << std::setw(12) << std::setiosflags(std::ios::left)
                << g[vertex].name << get(cm, vertex) << std::endl;
    }
    std::cout << "mean clustering coefficient: " << cc << std::endl;

    return 0;
}
//]
