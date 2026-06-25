// Strongly Connected Components example
#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/strong_components.hpp>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;

int main() {
    Graph g{5};
    // Cycle: 0->1->2->0, plus 2->3->4
    boost::add_edge(0, 1, g); boost::add_edge(1, 2, g); boost::add_edge(2, 0, g);
    boost::add_edge(2, 3, g); boost::add_edge(3, 4, g);

    std::vector<int> comp(boost::num_vertices(g));
    int n = boost::strong_components(g,
        boost::make_iterator_property_map(comp.begin(), boost::get(boost::vertex_index, g)));

    std::cout << "Number of strongly connected components: " << n << "\n";
    for (std::size_t v = 0; v < comp.size(); ++v) {
        std::cout << "  Vertex " << v << " -> component " << comp[v] << "\n";
    }
}
