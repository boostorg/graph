#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bipartite.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    Graph g(5);
    // Triangle 0-1-2 makes this non-bipartite
    boost::add_edge(0, 1, g); boost::add_edge(1, 2, g);
    boost::add_edge(2, 0, g); boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g);

    using Partition = boost::one_bit_color_map<
        boost::property_map<Graph, boost::vertex_index_t>::type>;
    Partition partition(num_vertices(g), get(boost::vertex_index, g));

    std::vector<int> cycle;
    boost::find_odd_cycle(g, get(boost::vertex_index, g), partition,
        std::back_inserter(cycle));

    if (cycle.empty()) {
        std::cout << "Graph is bipartite\n";
    } else {
        std::cout << "Odd cycle found:";
        for (auto v : cycle) std::cout << " " << v;
        std::cout << "\n";
    }
}
