#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_connectivity.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;
using Edge = graph_traits<Graph>::edge_descriptor;

int main() {
    Graph g(4);
    add_edge(0, 1, g); add_edge(0, 2, g);
    add_edge(1, 2, g); add_edge(1, 3, g); add_edge(2, 3, g);

    std::vector<Edge> cut_edges;
    auto connectivity = edge_connectivity(g, std::back_inserter(cut_edges));

    std::cout << "Edge connectivity: " << connectivity << "\n";
    std::cout << "Min cut edges: " << cut_edges.size() << "\n";
}
