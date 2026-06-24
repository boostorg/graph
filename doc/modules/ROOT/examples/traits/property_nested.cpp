// Nested `property<>` declarations let you stack multiple interior
// properties on a single kind (vertex, edge, or graph).
//
// New code should prefer bundled properties (structs) — see
// property_maps/bundled. Nested property<> is still needed when you
// want to coexist with algorithms that lookup a specific tag
// (e.g. edge_weight_t) OR when you need to declare vertex_index_t
// for a listS-based graph.

#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <string>

using namespace boost;

// Two edge properties: weight (int) nested inside a name (string).
using EdgeProps =
    property<edge_weight_t, int,
    property<edge_name_t,   std::string>>;

using Graph = adjacency_list<vecS, vecS, directedS, no_property, EdgeProps>;

int main() {
    Graph g(3);
    // Values are supplied positionally, outermost tag first.
    add_edge(0, 1, EdgeProps(5, property<edge_name_t, std::string>("a->b")), g);
    add_edge(1, 2, EdgeProps(3, property<edge_name_t, std::string>("b->c")), g);

    auto w = get(edge_weight, g);
    auto n = get(edge_name,   g);

    for (auto e : make_iterator_range(edges(g))) {
        std::cout << get(n, e) << " weight=" << get(w, e) << '\n';
    }
}
