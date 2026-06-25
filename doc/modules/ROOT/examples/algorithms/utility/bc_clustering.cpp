#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/connected_components.hpp>
#include <iostream>
#include <vector>

using namespace boost;

// Bundled edge scratch field holds the centrality value that the algorithm
// writes on each iteration. Passing it explicitly removes the need for the
// `edge_index_t` / iterator_property_map round-trip.
struct Edge { double centrality; };

using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;

int main() {
    Graph g(6);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(0, 2, g);  // cluster A
    add_edge(3, 4, g); add_edge(4, 5, g); add_edge(3, 5, g);  // cluster B
    add_edge(2, 3, g);                                         // bridge

    // 3-arg overload takes an explicit EdgeCentralityMap.
    betweenness_centrality_clustering(g,
        bc_clustering_threshold<double>(1.0, g, false),
        get(&Edge::centrality, g));

    std::vector<int> comp(num_vertices(g));
    int nc = connected_components(g, &comp[0]);
    std::cout << "Clusters: " << nc << "\n";
    for (std::size_t v = 0; v < comp.size(); ++v) {
        std::cout << "  vertex " << v << " -> cluster " << comp[v] << "\n";
    }
}
