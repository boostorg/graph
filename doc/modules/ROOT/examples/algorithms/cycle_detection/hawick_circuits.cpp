#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/hawick_circuits.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

struct circuit_printer {
    template <typename Vertices, typename G>
    void cycle(const Vertices& circuit, const G&) {
        for (auto v : circuit) { std::cout << v << " "; }
        std::cout << "\n";
    }
};

int main() {
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 0, g); add_edge(2, 3, g); add_edge(3, 0, g);

    std::cout << "Circuits:\n";
    circuit_printer visitor;
    hawick_circuits(g, visitor);
}
