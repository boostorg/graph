#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/king_ordering.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(6);
    add_edge(0, 3, g); add_edge(0, 5, g);
    add_edge(1, 2, g); add_edge(1, 4, g);
    add_edge(2, 5, g); add_edge(3, 4, g);

    std::vector<Vertex> order;
    king_ordering(g, std::back_inserter(order));

    std::cout << "King ordering:";
    for (auto v : order)
        std::cout << " " << v;
    std::cout << "\n";
}
