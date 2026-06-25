#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;

template <typename G>
typename graph_traits<G>::vertices_size_type  // note: typename required
count_vertices(const G& g) {
    return num_vertices(g);
}

int main() {
    using Graph = adjacency_list<vecS, vecS, undirectedS>;
    Graph g(5);
    std::cout << "|V| = " << count_vertices(g) << "\n";
}
