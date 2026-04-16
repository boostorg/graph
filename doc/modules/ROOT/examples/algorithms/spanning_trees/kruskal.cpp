#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <iostream>
#include <vector>

struct Road { int weight; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Road>;
    using Edge = graph_traits<Graph>::edge_descriptor;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(5);
    add_edge(0, 1, Road{2}, g);
    add_edge(0, 3, Road{1}, g);
    add_edge(1, 2, Road{3}, g);
    add_edge(1, 3, Road{2}, g);
    add_edge(2, 3, Road{5}, g);
    add_edge(2, 4, Road{4}, g);
    add_edge(3, 4, Road{6}, g);

    // Call the detail implementation directly to pass a bundled weight map
    auto n = num_vertices(g);
    std::vector<std::size_t> rank_vec(n);
    std::vector<Vertex> parent_vec(n);
    auto index = get(vertex_index, g);
    auto rank_map = make_iterator_property_map(rank_vec.begin(), index);
    auto parent_map = make_iterator_property_map(parent_vec.begin(), index);

    std::vector<Edge> mst;
    detail::kruskal_mst_impl(g, std::back_inserter(mst),
        rank_map, parent_map, get(&Road::weight, g));

    int total = 0;
    std::cout << "MST edges:\n";
    for (auto& e : mst) {
        int w = g[e].weight;
        std::cout << "  " << source(e, g) << " -- "
                  << target(e, g) << " (weight " << w << ")\n";
        total += w;
    }
    std::cout << "Total weight: " << total << "\n";
}
