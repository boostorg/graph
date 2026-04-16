#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/properties.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS,
    property<vertex_index_t, int>, property<edge_index_t, int>>;
using Edge = graph_traits<Graph>::edge_descriptor;

int main() {
    // A path graph: 0-1-2-3 (planar but not biconnected)
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(2, 3, g);

    // Assign edge indices
    int idx = 0;
    for (auto e : make_iterator_range(edges(g))) { put(edge_index, g, e, idx++); }

    // Compute planar embedding
    using EmbStorage = std::vector<std::vector<Edge>>;
    EmbStorage storage(num_vertices(g));
    auto embedding = make_iterator_property_map(storage.begin(), get(vertex_index, g));
    boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
        boyer_myrvold_params::embedding = embedding);

    std::cout << "Edges before: " << num_edges(g) << "\n";
    // Re-index edges before calling make_biconnected_planar
    idx = 0;
    for (auto e : make_iterator_range(edges(g))) { put(edge_index, g, e, idx++); }
    make_biconnected_planar(g, embedding, get(edge_index, g));
    std::cout << "Edges after make_biconnected_planar: " << num_edges(g) << "\n";
}
