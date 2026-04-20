#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/make_biconnected_planar.hpp>
#include <boost/graph/make_maximal_planar.hpp>
#include <iostream>
#include <vector>

using namespace boost;

struct Edge { int idx; };

using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;

void reindex(Graph& g) {
    int i = 0;
    for (auto e : make_iterator_range(edges(g))) { g[e].idx = i++; }
}

void reembed(Graph& g, std::vector<std::vector<Descriptor>>& s) {
    s.assign(num_vertices(g), {});
    auto emb = make_iterator_property_map(s.begin(), get(vertex_index, g));
    boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
        boyer_myrvold_params::embedding = emb);
}

int main() {
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(2, 3, g);

    std::vector<std::vector<Descriptor>> st(num_vertices(g));
    reindex(g); reembed(g, st);
    auto emb = make_iterator_property_map(st.begin(), get(vertex_index, g));
    make_biconnected_planar(g, emb, get(&Edge::idx, g));

    reindex(g); reembed(g, st);
    emb = make_iterator_property_map(st.begin(), get(vertex_index, g));
    std::cout << "Edges before: " << num_edges(g) << "\n";
    make_maximal_planar(g, emb, get(vertex_index, g), get(&Edge::idx, g));
    std::cout << "Edges after make_maximal_planar: " << num_edges(g) << "\n";
}
