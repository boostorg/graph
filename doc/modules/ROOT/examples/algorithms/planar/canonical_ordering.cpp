#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_canonical_ordering.hpp>
#include <iostream>
#include <vector>

using namespace boost;

struct Edge { int idx; };

using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;
using Vertex = graph_traits<Graph>::vertex_descriptor;
using Descriptor = graph_traits<Graph>::edge_descriptor;

int main() {
    // Start with a triangle (already maximal planar for 3 vertices)
    Graph g(3);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(0, 2, g);

    int i = 0;
    for (auto e : make_iterator_range(edges(g))) { g[e].idx = i++; }

    std::vector<std::vector<Descriptor>> storage(num_vertices(g));
    auto embedding = make_iterator_property_map(storage.begin(), get(vertex_index, g));
    boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
        boyer_myrvold_params::embedding = embedding);

    std::vector<Vertex> order;
    planar_canonical_ordering(g, embedding, std::back_inserter(order),
        get(vertex_index, g));

    std::cout << "Planar canonical ordering:";
    for (auto v : order) { std::cout << " " << v; }
    std::cout << "\n";
}
