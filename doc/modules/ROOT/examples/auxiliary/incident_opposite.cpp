#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;

    Graph g(4);
    auto e = add_edge(1, 3, g).first;

    // incident: get both endpoints of an edge
    auto endpoints = incident(e, g);
    std::cout << "incident(" << source(e, g) << "->" << target(e, g) << ") = ("
              << endpoints.first << ", " << endpoints.second << ")\n";

    // opposite: given one endpoint, get the other
    auto v = opposite(e, vertex(1, g), g);
    std::cout << "opposite(e, 1) = " << v << "\n";

    v = opposite(e, vertex(3, g), g);
    std::cout << "opposite(e, 3) = " << v << "\n";
}
