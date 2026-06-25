#include <boost/graph/adjacency_list.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<setS, vecS, directedS>;

    Graph g(1);
    add_edge(0, 3, g);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 2, g);  // duplicate; setS drops it

    std::cout << "out-edges of 0:";
    for (auto e : make_iterator_range(out_edges(0, g)))
        std::cout << " " << target(e, g);
    std::cout << "\n";
}
