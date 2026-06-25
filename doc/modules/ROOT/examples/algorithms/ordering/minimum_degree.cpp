#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/minimum_degree_ordering.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;
using size_type = graph_traits<Graph>::vertices_size_type;

int main() {
    int n = 6;
    Graph g(n);
    // Symmetric edges (required: both directions for each undirected edge)
    add_edge(0, 1, g); add_edge(1, 0, g);
    add_edge(0, 3, g); add_edge(3, 0, g);
    add_edge(1, 2, g); add_edge(2, 1, g);
    add_edge(2, 4, g); add_edge(4, 2, g);
    add_edge(3, 5, g); add_edge(5, 3, g);
    add_edge(4, 5, g); add_edge(5, 4, g);

    std::vector<int> inverse_perm(n), perm(n), degree(n), supernode(n, 1);
    auto id = get(vertex_index, g);

    minimum_degree_ordering(g,
        make_iterator_property_map(degree.begin(), id),
        make_iterator_property_map(inverse_perm.begin(), id),
        make_iterator_property_map(perm.begin(), id),
        make_iterator_property_map(supernode.begin(), id),
        0, id);

    std::cout << "Minimum degree ordering:";
    for (int i = 0; i < n; ++i)
        std::cout << " " << inverse_perm[i];
    std::cout << "\n";
}
