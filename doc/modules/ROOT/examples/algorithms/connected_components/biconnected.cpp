#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS,
        no_property, property<edge_index_t, int>>;

    Graph g(5);
    add_edge(0, 1, 0, g);
    add_edge(1, 2, 1, g);
    add_edge(2, 0, 2, g);  // triangle
    add_edge(2, 3, 3, g);
    add_edge(3, 4, 4, g);
    add_edge(4, 2, 5, g);  // second triangle

    std::vector<std::size_t> comp(num_edges(g));
    auto comp_map = make_iterator_property_map(comp.begin(), get(edge_index, g));
    auto num = biconnected_components(g, comp_map);

    std::cout << "Biconnected components: " << num << "\n";
    for (auto ei = edges(g).first; ei != edges(g).second; ++ei) {
        std::cout << "  " << source(*ei, g) << "-" << target(*ei, g)
                  << " component " << get(comp_map, *ei) << "\n";
    }
}
