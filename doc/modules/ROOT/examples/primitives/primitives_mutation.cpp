#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;

int main() {
    Graph g(2);
    add_edge(0, 1, g);

    auto v = add_vertex(g);
    add_edge(v, vertex(0, g), g);
    clear_vertex(v, g);    // remove all edges of v
    remove_vertex(v, g);   // remove v itself

    std::cout << "vertices: " << num_vertices(g) << "\n";
}
