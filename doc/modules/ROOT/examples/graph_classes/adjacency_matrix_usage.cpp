#include <boost/graph/adjacency_matrix.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <string>

struct City { std::string name; };
struct Road { double km; };

int main() {
    using namespace boost;
    using Graph = adjacency_matrix<directedS, City, Road>;

    // Vertex count is fixed at construction
    Graph g(4);
    g[0].name = "Paris";
    g[1].name = "Lyon";
    g[2].name = "Marseille";
    g[3].name = "Nice";

    add_edge(0, 1, Road{460}, g);
    add_edge(0, 2, Road{775}, g);
    add_edge(1, 2, Road{310}, g);
    add_edge(2, 3, Road{200}, g);

    // O(1) edge lookup
    auto result = edge(0, 1, g);
    if (result.second) {
        std::cout << "Paris -> Lyon: " << g[result.first].km << " km\n";
    }

    // Traverse
    for (auto v : make_iterator_range(vertices(g))) {
        for (auto e : make_iterator_range(out_edges(v, g))) {
            std::cout << g[source(e, g)].name << " -> "
                      << g[target(e, g)].name
                      << " (" << g[e].km << " km)\n";
        }
    }
}
