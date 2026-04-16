// Bipartite Check example
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bipartite.hpp>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;

int main() {
    // Bipartite graph: {0,2} and {1,3} with edges only between sets
    Graph bipartite_g{4};
    boost::add_edge(0, 1, bipartite_g); boost::add_edge(0, 3, bipartite_g);
    boost::add_edge(2, 1, bipartite_g); boost::add_edge(2, 3, bipartite_g);

    // Non-bipartite graph: triangle
    Graph triangle{3};
    boost::add_edge(0, 1, triangle); boost::add_edge(1, 2, triangle);
    boost::add_edge(0, 2, triangle);

    std::cout << "4-cycle is bipartite: "
              << (boost::is_bipartite(bipartite_g) ? "yes" : "no") << "\n";
    std::cout << "Triangle is bipartite: "
              << (boost::is_bipartite(triangle) ? "yes" : "no") << "\n";
}
