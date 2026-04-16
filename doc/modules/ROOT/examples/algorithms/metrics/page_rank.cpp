#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/page_rank.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;

    // A small web graph: 0 -> 1, 0 -> 2, 1 -> 2, 2 -> 0, 3 -> 2
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 2, g);
    add_edge(2, 0, g);
    add_edge(3, 2, g);

    std::vector<double> ranks(num_vertices(g));
    auto rank_map = make_iterator_property_map(
        ranks.begin(), get(vertex_index, g));

    // Run PageRank with default damping=0.85 and 20 iterations
    graph::page_rank(g, rank_map);

    std::cout << std::fixed << std::setprecision(4);
    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v << "  rank=" << ranks[v] << "\n";
    }
}
