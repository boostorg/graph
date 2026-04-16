#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    // MAS requires edge_weight_t internal property
    using Graph = adjacency_list<vecS, vecS, undirectedS,
        no_property, property<edge_weight_t, int>>;

    Graph g(5);
    add_edge(0, 1, 2, g);
    add_edge(0, 4, 3, g);
    add_edge(1, 2, 3, g);
    add_edge(1, 4, 2, g);
    add_edge(2, 3, 4, g);
    add_edge(3, 4, 1, g);

    std::vector<int> weights(num_vertices(g));
    auto weight_map = make_iterator_property_map(
        weights.begin(), get(vertex_index, g));

    maximum_adjacency_search(g,
        boost::weight_map(get(edge_weight, g)));

    std::cout << "Maximum adjacency search completed\n";
    std::cout << "Last vertex visited has highest connectivity\n";
}
