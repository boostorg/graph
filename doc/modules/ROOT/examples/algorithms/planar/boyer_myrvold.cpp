#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

Graph make_complete(int n) {
    Graph g(n);
    for (int i = 0; i < n; ++i)
        for (int j = i + 1; j < n; ++j)
            add_edge(i, j, g);
    return g;
}

int main() {
    auto k4 = make_complete(4);
    auto k5 = make_complete(5);
    std::cout << "K4 is planar: " << std::boolalpha
              << boyer_myrvold_planarity_test(k4) << "\n";
    std::cout << "K5 is planar: " << std::boolalpha
              << boyer_myrvold_planarity_test(k5) << "\n";
}
