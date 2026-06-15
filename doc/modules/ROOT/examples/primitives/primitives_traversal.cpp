#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;

int main() {
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);

    for (auto v : make_iterator_range(vertices(g))) {
        for (auto e : make_iterator_range(out_edges(v, g))) {
            std::cout << source(e, g) << " -> " << target(e, g) << "\n";
        }
    }
}
