#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

struct Counter : default_bfs_visitor {
    int* n;  // indirection so mutations escape the copy
    explicit Counter(int* p) : n(p) {}
    template <typename V, typename G>
    void discover_vertex(V, const G&) const { ++*n; }
};

int main() {
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g); add_edge(2, 3, g);

    int n = 0;
    breadth_first_search(g, 0, visitor(Counter{&n}));
    std::cout << "discovered " << n << " vertices\n";
}
