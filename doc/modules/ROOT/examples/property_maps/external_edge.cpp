#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>

int main() {
    using namespace boost;

    // Graph with edge_index as an internal property
    using Graph = adjacency_list<vecS, vecS, directedS,
                                 no_property,
                                 property<edge_index_t, std::size_t>>;

    Graph g(4);
    // Assign each edge an index manually
    add_edge(0, 1, 0, g);
    add_edge(0, 2, 1, g);
    add_edge(1, 2, 2, g);
    add_edge(1, 3, 3, g);
    add_edge(2, 3, 4, g);

    // External arrays indexed by edge ID
    int capacity[] = {10, 20, 30, 40, 50};
    int flow[]     = { 8, 12, 25, 32, 45};

    auto edge_id = get(edge_index, g);
    auto cap_map  = make_iterator_property_map(capacity, edge_id);
    auto flow_map = make_iterator_property_map(flow, edge_id);

    for (auto ei = edges(g).first; ei != edges(g).second; ++ei) {
        std::cout << source(*ei, g) << " -> " << target(*ei, g)
                  << "  capacity=" << get(cap_map, *ei)
                  << "  flow=" << get(flow_map, *ei) << "\n";
    }
}
