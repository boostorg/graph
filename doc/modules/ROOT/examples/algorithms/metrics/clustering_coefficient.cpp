#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/clustering_coefficient.hpp>
#include <iostream>
#include <vector>

// clustering_coefficient requires AdjacencyMatrix concept (lookup_edge),
// so we use adjacency_matrix instead of adjacency_list.
using Graph = boost::adjacency_matrix<boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{5};
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g); // triangle on {0,1,2}
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g);

    for (auto v : boost::make_iterator_range(vertices(g))) {
        double cc = boost::clustering_coefficient(g, v);
        std::cout << "vertex " << v << ": clustering coeff = " << cc << "\n";
    }
}
