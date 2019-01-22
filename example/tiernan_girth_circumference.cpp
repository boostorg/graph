// (C) Copyright Andrew Sutton 2007
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

//[tiernan_girth_circumference
#include <iostream>

#include <boost/graph/directed_graph.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>

#include "helper.hpp"

using namespace boost;

// Declare the graph type and its vertex and edge types.
using Graph = directed_graph<>;
using Vertex = graph_traits<Graph>::vertex_descriptor;
using Edge = graph_traits<Graph>::edge_descriptor;

int
main(int argc, char *argv[])
{
    // Create the graph and read it from standard input.
    Graph g;
    read_graph(g, std::cin);

    // Compute the girth and circumference simulataneously
    auto [girth, circ] = tiernan_girth_and_circumference(g);

    // Print the result
    std::cout << "girth: " << girth << std::endl;
    std::cout << "circumference: " << circ << std::endl;

    return 0;
}
//]
