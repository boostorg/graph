#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/howard_cycle_ratio.hpp>
#include <iostream>
#include <vector>

// Two bundled edge fields replace the nested
// `property<edge_weight_t, double, property<edge_weight2_t, double>>`.
struct Edge { double weight, transit; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;

int main() {
    Graph g(3);
    // Cycle: 0->1->2->0 with weights 3,1,2 and transit times 1,1,1
    add_edge(0, 1, Edge{3.0, 1.0}, g);
    add_edge(1, 2, Edge{1.0, 1.0}, g);
    add_edge(2, 0, Edge{2.0, 1.0}, g);
    add_edge(0, 2, Edge{10.0, 1.0}, g);

    std::vector<graph_traits<Graph>::edge_descriptor> cycle;
    double mcr = minimum_cycle_ratio(g,
        get(vertex_index, g),
        get(&Edge::weight, g),
        get(&Edge::transit, g), &cycle);

    std::cout << "Minimum cycle ratio: " << mcr << "\n";
    std::cout << "Cycle length: " << cycle.size() << " edges\n";
}
