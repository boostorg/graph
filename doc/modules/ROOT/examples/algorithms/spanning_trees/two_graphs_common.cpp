#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/two_graphs_common_spanning_trees.hpp>
#include <iostream>
#include <vector>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Edge = boost::graph_traits<Graph>::edge_descriptor;

int count = 0;

struct CountTrees {
    void operator()(const std::vector<bool>& inL) {
        ++count;
        std::cout << "  Tree " << count << ": edges";
        for (std::size_t i = 0; i < inL.size(); ++i) {
            if (inL[i]) { std::cout << " " << i; }
        }
        std::cout << "\n";
    }
};

int main() {
    // Two identical triangle graphs (3 vertices, 3 edges)
    Graph g1(3), g2(3);
    boost::add_edge(0, 1, g1); boost::add_edge(1, 2, g1); boost::add_edge(0, 2, g1);
    boost::add_edge(0, 1, g2); boost::add_edge(1, 2, g2); boost::add_edge(0, 2, g2);

    std::vector<bool> inL(num_edges(g1), false);
    std::cout << "Common spanning trees:\n";
    boost::two_graphs_common_spanning_trees(g1, g2, CountTrees{}, inL);
    std::cout << "Total: " << count << "\n";
}
