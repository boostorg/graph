#include <boost/graph/adjacency_list.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<listS, listS, undirectedS>;
    Graph g(3);
    auto it = vertices(g).first;
    auto a = *it, b = *++it, c = *++it;
    add_edge(a, b, g);
    add_edge(b, c, g);
    add_edge(a, c, g);

    std::cout << "before: " << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";

    clear_vertex(b, g);    // drop all edges incident to b
    remove_vertex(b, g);   // now safe to erase

    std::cout << "after:  " << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";
}
