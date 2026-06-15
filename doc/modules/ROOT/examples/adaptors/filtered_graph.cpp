#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <functional>
#include <iostream>

struct Road {
    int weight;
};

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property, Road>;
    using Edge = graph_traits<Graph>::edge_descriptor;

    Graph g(4);
    add_edge(0, 1, Road{5}, g);
    add_edge(0, 2, Road{0}, g);
    add_edge(1, 3, Road{3}, g);
    add_edge(2, 3, Road{0}, g);

    // Keep only edges with positive weight
    auto filter = [&g](Edge e) { return g[e].weight > 0; };
    filtered_graph<Graph, std::function<bool(Edge)>> fg(g, filter);

    std::cout << "Original: " << num_edges(g) << " edges\n";
    for (auto ei = edges(g).first; ei != edges(g).second; ++ei) {
        std::cout << "  " << source(*ei, g) << " -> " << target(*ei, g)
                  << " (weight=" << g[*ei].weight << ")\n";
    }

    std::cout << "Filtered (weight > 0):\n";
    for (auto ei = edges(fg).first; ei != edges(fg).second; ++ei) {
        std::cout << "  " << source(*ei, fg) << " -> " << target(*ei, fg)
                  << " (weight=" << fg[*ei].weight << ")\n";
    }
}
