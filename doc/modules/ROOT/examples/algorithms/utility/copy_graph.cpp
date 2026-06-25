#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
    Graph g;
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);

    Graph g_copy;
    boost::copy_graph(g, g_copy);

    std::cout << "Original: " << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";
    std::cout << "Copy:     " << num_vertices(g_copy) << " vertices, "
              << num_edges(g_copy) << " edges\n";
}
