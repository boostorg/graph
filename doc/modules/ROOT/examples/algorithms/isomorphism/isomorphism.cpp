#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/isomorphism.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
    Graph g1, g2;
    // Triangle: 0->1->2->0
    boost::add_edge(0, 1, g1); boost::add_edge(1, 2, g1); boost::add_edge(2, 0, g1);
    // Same triangle, relabeled: 2->0->1->2
    boost::add_edge(2, 0, g2); boost::add_edge(0, 1, g2); boost::add_edge(1, 2, g2);

    bool result = boost::isomorphism(g1, g2);
    std::cout << "Graphs are " << (result ? "isomorphic" : "not isomorphic") << "\n";
}
