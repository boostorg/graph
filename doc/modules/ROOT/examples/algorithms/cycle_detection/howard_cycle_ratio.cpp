#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/howard_cycle_ratio.hpp>
#include <iostream>
#include <vector>

// howard_cycle_ratio requires property-tag-based weight maps.
using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, no_property,
    property<edge_weight_t, double, property<edge_weight2_t, double>>>;

int main() {
    Graph g(3);
    // Cycle: 0->1->2->0 with weights 3,1,2 and transit times 1,1,1
    add_edge(0, 1, {3.0, 1.0}, g);
    add_edge(1, 2, {1.0, 1.0}, g);
    add_edge(2, 0, {2.0, 1.0}, g);
    add_edge(0, 2, {10.0, 1.0}, g);

    std::vector<graph_traits<Graph>::edge_descriptor> cycle;
    double mcr = minimum_cycle_ratio(g,
        get(vertex_index, g),
        get(edge_weight, g),
        get(edge_weight2, g), &cycle);

    std::cout << "Minimum cycle ratio: " << mcr << "\n";
    std::cout << "Cycle length: " << cycle.size() << " edges\n";
}
