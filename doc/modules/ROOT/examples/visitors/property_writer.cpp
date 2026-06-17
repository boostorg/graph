#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <iterator>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS>;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 4, g);

    // Print vertex indices as they are discovered
    std::ostream_iterator<int> out(std::cout, " ");
    auto vis = make_bfs_visitor(
        write_property(get(vertex_index, g), out, on_discover_vertex())
    );

    std::cout << "BFS discovery order: ";
    breadth_first_search(g, vertex(0, g), visitor(vis));
    std::cout << "\n";
}
