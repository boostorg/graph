#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <iostream>
#include <vector>

struct Road { int weight; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Road>;
    using Edge  = graph_traits<Graph>::edge_descriptor;

    Graph g(5);
    add_edge(0, 1, Road{2}, g);
    add_edge(0, 3, Road{1}, g);
    add_edge(1, 2, Road{3}, g);
    add_edge(1, 3, Road{2}, g);
    add_edge(2, 3, Road{5}, g);
    add_edge(2, 4, Road{4}, g);
    add_edge(3, 4, Road{6}, g);

    // Named-parameter overload: needed because the weight lives in a bundled
    // property (not the interior `edge_weight_t` tag that the 2-arg positional
    // overload would pick up).
    std::vector<Edge> mst;
    kruskal_minimum_spanning_tree(g, std::back_inserter(mst),
        weight_map(get(&Road::weight, g)));

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
