// Demonstrates property_map<Graph, Tag>::type vs ::const_type.
//
// Rule of thumb: if your Graph& is const, you get ::const_type; otherwise
// you get ::type. Deducing it via `auto` (C++14) sidesteps the choice
// entirely and is what most code should do.

#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, int>>;

// Mutable graph: ::type works.
void print_weights(Graph& g) {
    using PMap = property_map<Graph, edge_weight_t>::type;
    PMap w = get(edge_weight, g);
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "(" << source(e, g) << "," << target(e, g)
                  << ") weight=" << get(w, e) << '\n';
    }
}

// Const graph: ::const_type is required.
int total_weight(const Graph& g) {
    using PMap = property_map<Graph, edge_weight_t>::const_type;
    PMap w = get(edge_weight, g);
    int total = 0;
    for (auto e : make_iterator_range(edges(g))) total += get(w, e);
    return total;
}

int main() {
    Graph g(3);
    add_edge(0, 1, 5, g);
    add_edge(1, 2, 3, g);
    add_edge(0, 2, 8, g);

    print_weights(g);
    std::cout << "total = " << total_weight(g) << '\n';
}
