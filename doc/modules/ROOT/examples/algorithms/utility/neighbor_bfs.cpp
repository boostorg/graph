#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/neighbor_bfs.hpp>
#include <iostream>

struct print_visitor : public boost::neighbor_bfs_visitor<> {
    template <typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph&) const {
        std::cout << v << " ";
    }
};

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, bidirectionalS>;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(2, 1, g);
    add_edge(3, 0, g);
    add_edge(1, 4, g);

    std::cout << "Neighbor BFS from vertex 1: ";
    neighbor_breadth_first_search(g, vertex(1, g),
        visitor(print_visitor()));
    std::cout << "\n";
}
