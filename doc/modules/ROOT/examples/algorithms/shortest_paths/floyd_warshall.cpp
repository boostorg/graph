#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <iostream>
#include <limits>
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

    floyd_warshall_all_pairs_shortest_paths(g, D,
        get(&Edge::weight, g),
        std::less<int>{},
        closed_plus<int>{(std::numeric_limits<int>::max)()},
        (std::numeric_limits<int>::max)(),
        int{0});

    for (std::size_t i = 0; i < n; ++i) {
        for (std::size_t j = 0; j < n; ++j)
            std::cout << D[i][j] << (j + 1 < n ? " " : "");
        std::cout << "\n";
    }
}
