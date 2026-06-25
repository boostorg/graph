#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

// Inherit from default_bfs_visitor to get empty defaults for all events.
// Override only the events you care about.
struct my_visitor : public boost::default_bfs_visitor {
    template <typename Vertex, typename Graph>
    void discover_vertex(Vertex v, const Graph&) const {
        std::cout << "  discovered " << v << "\n";
    }

    template <typename Edge, typename Graph>
    void tree_edge(Edge e, const Graph& g) const {
        std::cout << "  tree edge " << source(e, g)
                  << " -> " << target(e, g) << "\n";
    }
};

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS>;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 4, g);

    std::cout << "BFS from vertex 0:\n";
    breadth_first_search(g, vertex(0, g), visitor(my_visitor()));
}
