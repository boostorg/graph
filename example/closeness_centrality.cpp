// (C) Copyright Andrew Sutton 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

//[closeness_centrality_example
#include <iostream>
#include <iomanip>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/closeness_centrality.hpp>
#include <boost/graph/property_maps/constant_property_map.hpp>
#include "helper.hpp"

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

// Declare a matrix type and its corresponding property map that
// will contain the distances between each pair of vertices.
using DistanceProperty = exterior_vertex_property<Graph, int>;
using DistanceMatrix = DistanceProperty::matrix_type;
using DistanceMatrixMap = DistanceProperty::matrix_map_type;

// Declare the weight map so that each edge returns the same value.
using WeightMap = constant_property_map<Edge, int>;

// Declare a container and its corresponding property map that
// will contain the resulting closeness centralities of each
// vertex in the graph.
using ClosenessProperty = boost::exterior_vertex_property<Graph, float>;
using ClosenessContainer = ClosenessProperty::container_type;
using ClosenessMap = ClosenessProperty::map_type;

int
main(int argc, char *argv[])
{
    // Create the graph and a property map that provides access to[
    // tha actor names.
    Graph g;
    NameMap nm(get(&Actor::name, g));

    // Read the graph from standard input.
    read_graph(g, nm, std::cin);

    // Compute the distances between all pairs of vertices using
    // the Floyd-Warshall algorithm. Note that the weight map is
    // created so that every edge has a weight of 1.
    DistanceMatrix distances(num_vertices(g));
    DistanceMatrixMap dm(distances, g);
    WeightMap wm(1);
    floyd_warshall_all_pairs_shortest_paths(g, dm, weight_map(wm));

    // Compute the closeness centrality for graph.
    ClosenessContainer cents(num_vertices(g));
    ClosenessMap cm(cents, g);
    all_closeness_centralities(g, dm, cm);

    // Print the closeness centrality of each vertex.
    graph_traits<Graph>::vertex_iterator i, end;
    for(std::tie(i, end) = vertices(g); i != end; ++i) {
          std::cout << std::setw(12) << std::setiosflags(std::ios::left)
             << g[*i].name << get(cm, *i) << std::endl;
    }

    return 0;
}
//]
