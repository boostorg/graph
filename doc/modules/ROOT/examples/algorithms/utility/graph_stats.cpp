#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_stats.hpp>
#include <iostream>
#include <map>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;

int main() {
    Graph g(4);
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(0, 1, g); // duplicate edge

    unsigned long dups = boost::graph::num_dup_edges(g);
    std::cout << "Duplicate edges: " << dups << "\n";

    auto dist = boost::graph::degree_dist(g);
    std::cout << "Degree distribution:\n";
    for (auto& p : dist) {
        std::cout << "  degree " << p.first << ": " << p.second << " vertices\n";
    }
}
