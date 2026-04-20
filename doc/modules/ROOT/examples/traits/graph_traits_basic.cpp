#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;
using Traits = graph_traits<Graph>;

int main() {
    Graph g(3);
    add_edge(0, 1, g);
    add_edge(1, 2, g);

    // Descriptors and sizes come out of graph_traits
    Traits::vertex_descriptor s = vertex(0, g);
    Traits::vertices_size_type n = num_vertices(g);

    std::cout << "num_vertices = " << n << '\n';
    std::cout << "source vertex = " << s << '\n';
    std::cout << "is_directed   = " << std::boolalpha << is_directed(g) << '\n';

    // The null vertex sentinel — portable across selectors
    Traits::vertex_descriptor nv = Traits::null_vertex();
    std::cout << "null_vertex   = " << nv << '\n';
}
