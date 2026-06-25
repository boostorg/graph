#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transpose_graph.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
    Graph g;
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 0, g);

    std::cout << "Original edges: ";
    for (auto ep = edges(g); ep.first != ep.second; ++ep.first) {
        std::cout << source(*ep.first, g) << "->" << target(*ep.first, g) << " ";
    }

    Graph gt;
    boost::transpose_graph(g, gt);

    std::cout << "\nTransposed edges: ";
    for (auto ep = edges(gt); ep.first != ep.second; ++ep.first) {
        std::cout << source(*ep.first, gt) << "->" << target(*ep.first, gt) << " ";
    }
    std::cout << "\n";
}
