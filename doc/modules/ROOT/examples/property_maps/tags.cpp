#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, double>>;

int main() {
    Graph g;
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 0, g);
    add_edge(0, 2, g);

    // randomize_property requires tag-based internal properties
    mt19937 gen(42);
    randomize_property<edge_weight_t>(g, gen);

    // obtain the property map via the tag
    auto weight = get(edge_weight, g);

    graph_traits<Graph>::edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        std::cout << source(*ei, g) << " -> " << target(*ei, g)
                  << "  weight=" << get(weight, *ei) << "\n";
    }
}
