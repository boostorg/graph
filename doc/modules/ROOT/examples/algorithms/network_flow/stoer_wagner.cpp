#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>

struct EdgeProps { int weight; };

using namespace boost;

using Graph = adjacency_list<vecS, vecS, undirectedS,
    no_property, EdgeProps>;

int main() {
    Graph g{4};
    add_edge(0, 1, EdgeProps{2}, g);
    add_edge(0, 2, EdgeProps{3}, g);
    add_edge(1, 2, EdgeProps{3}, g);
    add_edge(1, 3, EdgeProps{2}, g);
    add_edge(2, 3, EdgeProps{4}, g);

    auto weight_map = get(&EdgeProps::weight, g);
    int cut = stoer_wagner_min_cut(g, weight_map);
    std::cout << "Stoer-Wagner min cut: " << cut << "\n";
}
