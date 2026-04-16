#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    Graph g{6};
    boost::add_edge(0, 3, g);
    boost::add_edge(0, 5, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(1, 4, g);
    boost::add_edge(2, 5, g);
    boost::add_edge(3, 4, g);

    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    std::vector<Vertex> order;
    boost::cuthill_mckee_ordering(g, std::back_inserter(order));

    std::cout << "Cuthill-McKee ordering:";
    for (auto v : order) {
        std::cout << " " << v;
    }
    std::cout << "\n";
}
