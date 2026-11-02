#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <functional>
#include <iostream>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;

struct Counter : boost::default_bfs_visitor {
    int n = 0;
    template <typename V, typename G>
    void discover_vertex(V, const G&) { ++n; }
};

int main() {
    Graph g(4);
    boost::add_edge(0, 1, g); boost::add_edge(1, 2, g); boost::add_edge(2, 3, g);

    Counter counter;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::breadth_first_search(g, 0, boost::visitor(std::ref(counter)));

    std::cout << "discovered " << counter.n << " vertices\n";
}
