#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, bidirectionalS>;

int main() {
    Graph g(3);
    add_edge(0, 1, g);
    add_edge(2, 1, g);

    std::cout << "num_vertices: " << num_vertices(g) << "\n";
    std::cout << "num_edges:    " << num_edges(g) << "\n";
    std::cout << "out_degree(0): " << out_degree(0, g) << "\n";
    std::cout << "in_degree(1):  " << in_degree(1, g) << "\n";
    std::cout << "degree(1):     " << degree(1, g) << "\n";
}
