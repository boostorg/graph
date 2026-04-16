#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/make_connected.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

int main() {
    Graph g(6);
    add_edge(0, 1, g);
    add_edge(2, 3, g);
    add_edge(4, 5, g);

    std::cout << "Edges before: " << num_edges(g) << "\n";
    make_connected(g);
    std::cout << "Edges after:  " << num_edges(g) << "\n";
}
