#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/smallest_last_ordering.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <iostream>
#include <vector>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(5);
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g);

    auto order = boost::smallest_last_vertex_ordering(g);
    std::cout << "Smallest-last ordering:\n";
    for (std::size_t i = 0; i < order.size(); ++i) {
        std::cout << "  position " << i << ": vertex " << order[i] << "\n";
    }
}
