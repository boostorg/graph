#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bc_clustering.hpp>
#include <boost/graph/connected_components.hpp>
#include <iostream>
#include <vector>

using namespace boost;

using Graph = adjacency_list<vecS, vecS, undirectedS,
    no_property, property<edge_index_t, int>>;

int main() {
    Graph g(6);
    int idx = 0;
    auto ae = [&](int u, int v) { add_edge(u, v, {idx++}, g); };
    ae(0,1); ae(1,2); ae(0,2);   // cluster A
    ae(3,4); ae(4,5); ae(3,5);   // cluster B
    ae(2,3);                      // bridge

    betweenness_centrality_clustering(g, bc_clustering_threshold<double>(1.0, g, false));

    std::vector<int> comp(num_vertices(g));
    int nc = connected_components(g, &comp[0]);
    std::cout << "Clusters: " << nc << "\n";
    for (std::size_t v = 0; v < comp.size(); ++v) {
        std::cout << "  vertex " << v << " -> cluster " << comp[v] << "\n";
    }
}
