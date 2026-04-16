#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/profile.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    Graph g{5};
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(0, 4, g);
    boost::add_edge(3, 4, g);

    auto p = boost::profile(g);
    std::cout << "Profile: " << p << "\n";
}
