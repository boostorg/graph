#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;

int main() {
    Graph g(3);
    add_edge(0, 1, g);

    auto result = edge(0, 1, g);       // pair of (edge_descriptor, bool)
    if (result.second) {
        std::cout << source(result.first, g) << " -> " << target(result.first, g) << "\n";
    }
}
