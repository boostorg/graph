#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/st_connected.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

int main() {
    Graph g(5);
    add_edge(0, 1, g); add_edge(1, 2, g);
    // vertices 3 and 4 are isolated from 0-1-2

    bool connected_0_2 = graph::st_connected(g, vertex(0, g), vertex(2, g));
    bool connected_0_3 = graph::st_connected(g, vertex(0, g), vertex(3, g));

    std::cout << "0 connected to 2: " << std::boolalpha << connected_0_2 << "\n";
    std::cout << "0 connected to 3: " << std::boolalpha << connected_0_3 << "\n";
}
