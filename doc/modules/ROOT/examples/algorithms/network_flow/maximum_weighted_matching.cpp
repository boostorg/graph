#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_weighted_matching.hpp>
#include <iostream>
#include <vector>

using namespace boost;

// This algorithm hard-codes its weight source to the interior `edge_weight_t`
// tag (see boost/graph/maximum_weighted_matching.hpp). Bundled properties are
// not supported here; the tag-based form below is required.
using Graph = adjacency_list<vecS, vecS, undirectedS, no_property,
    property<edge_weight_t, int>>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(5);
    add_edge(0, 1, {3}, g);
    add_edge(1, 2, {1}, g);
    add_edge(2, 3, {4}, g);
    add_edge(3, 4, {2}, g);
    add_edge(0, 4, {5}, g);

    std::vector<Vertex> mate(num_vertices(g));
    maximum_weighted_matching(g, &mate[0]);

    std::cout << "Matched edges:\n";
    int total = 0;
    for (Vertex v = 0; v < num_vertices(g); ++v) {
        if (mate[v] != graph_traits<Graph>::null_vertex() && v < mate[v]) {
            auto w = get(edge_weight, g, edge(v, mate[v], g).first);
            std::cout << "  " << v << " -- " << mate[v] << " (w=" << w << ")\n";
            total += w;
        }
    }
    std::cout << "Total weight: " << total << "\n";
}
