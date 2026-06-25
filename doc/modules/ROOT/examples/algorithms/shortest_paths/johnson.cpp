#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/johnson_all_pairs_shortest.hpp>
#include <iostream>
#include <vector>

struct Node {};
struct Edge { int weight; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, Node, Edge>;

int main() {
    Graph g{3};
    add_edge(0, 1, Edge{2}, g);
    add_edge(1, 2, Edge{3}, g);
    add_edge(0, 2, Edge{7}, g);

    auto n = num_vertices(g);
    std::vector<std::vector<int>> D(n, std::vector<int>(n));

    johnson_all_pairs_shortest_paths(g, D,
        get(vertex_index, g), get(&Edge::weight, g), int{0});

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j)
            std::cout << D[i][j] << (j + 1 < n ? " " : "");
        std::cout << "\n";
    }
}
