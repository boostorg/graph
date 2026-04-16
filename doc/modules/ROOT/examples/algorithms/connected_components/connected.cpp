// Connected Components example
#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;

int main() {
    Graph g{7};
    // Component 0: {0,1,2}  Component 1: {3,4}  Component 2: {5,6}
    boost::add_edge(0, 1, g); boost::add_edge(1, 2, g);
    boost::add_edge(3, 4, g);
    boost::add_edge(5, 6, g);

    std::vector<int> comp(boost::num_vertices(g));
    int n = boost::connected_components(g,
        boost::make_iterator_property_map(comp.begin(), boost::get(boost::vertex_index, g)));

    std::cout << "Number of components: " << n << "\n";
    for (std::size_t v = 0; v < comp.size(); ++v) {
        std::cout << "  Vertex " << v << " -> component " << comp[v] << "\n";
    }
}
