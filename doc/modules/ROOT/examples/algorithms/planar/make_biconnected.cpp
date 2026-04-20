#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <iostream>
#include <vector>

using namespace boost;

// Bundled edge field holds the contiguous index that planar algorithms need.
struct Edge { int idx; };

using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;

void reindex(Graph& g) {
    int i = 0;
    for (auto e : make_iterator_range(edges(g))) { g[e].idx = i++; }
}

int main() {
    // A path graph: 0-1-2-3 (planar but not biconnected)
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(2, 3, g);

    reindex(g);

    // Compute planar embedding
    using EmbStorage = std::vector<std::vector<Descriptor>>;
    EmbStorage storage(num_vertices(g));
    auto embedding = make_iterator_property_map(storage.begin(), get(vertex_index, g));
    boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
        boyer_myrvold_params::embedding = embedding);

    std::cout << "Edges before: " << num_edges(g) << "\n";
    reindex(g);
    make_biconnected_planar(g, embedding, get(&Edge::idx, g));
    std::cout << "Edges after make_biconnected_planar: " << num_edges(g) << "\n";
}
