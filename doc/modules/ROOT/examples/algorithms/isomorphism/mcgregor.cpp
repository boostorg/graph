#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/mcgregor_common_subgraphs.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;
using Size = graph_traits<Graph>::vertices_size_type;

struct max_callback {
    Size largest = 0;
    template <typename Map1, typename Map2>
    bool operator()(Map1, Map2, Size subgraph_size) {
        if (subgraph_size > largest) largest = subgraph_size;
        return true;
    }
};

int main() {
    Graph g1(4);
    add_edge(0, 1, g1); add_edge(1, 2, g1);
    add_edge(2, 3, g1); add_edge(3, 0, g1);

    Graph g2(5);
    add_edge(0, 1, g2); add_edge(1, 2, g2);
    add_edge(2, 3, g2); add_edge(3, 4, g2); add_edge(4, 0, g2);

    max_callback cb;
    mcgregor_common_subgraphs(g1, g2, true, std::ref(cb));
    std::cout << "Largest common subgraph size: " << cb.largest << "\n";
}
