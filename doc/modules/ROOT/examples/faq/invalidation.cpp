#include <boost/graph/adjacency_list.hpp>
#include <iostream>

int main() {
    using namespace boost;

    // vecS: descriptors are indices into a vector. Removing 1 shifts 2 -> 1.
    using Vec = adjacency_list<vecS, vecS, directedS>;
    Vec g(3);
    std::cout << "before remove: ";
    for (auto v : make_iterator_range(vertices(g))) std::cout << v << " ";
    remove_vertex(1, g);
    std::cout << "\nafter  remove: ";
    for (auto v : make_iterator_range(vertices(g))) std::cout << v << " ";
    std::cout << "\n";
    // What used to be vertex 2 is now vertex 1. Any saved descriptor is stale.
}
