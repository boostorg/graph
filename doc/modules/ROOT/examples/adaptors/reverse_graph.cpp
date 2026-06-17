#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, bidirectionalS>;

    Graph g(4);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    add_edge(3, 0, g);

    std::cout << "Original:\n";
    print_graph(g);

    std::cout << "\nReversed:\n";
    print_graph(make_reverse_graph(g));
}
