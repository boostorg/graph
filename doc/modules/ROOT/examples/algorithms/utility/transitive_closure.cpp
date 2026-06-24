#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transitive_closure.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
    Graph g;
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);

    Graph tc;
    boost::transitive_closure(g, tc);

    std::cout << "Transitive closure edges:\n";
    for (auto ep = edges(tc); ep.first != ep.second; ++ep.first) {
        std::cout << "  " << source(*ep.first, tc) << " -> " << target(*ep.first, tc) << "\n";
    }
}
