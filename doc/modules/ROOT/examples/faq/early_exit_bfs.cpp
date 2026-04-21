#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

struct Found {};

struct StopAt : default_bfs_visitor {
    Vertex target;
    explicit StopAt(Vertex t) : target(t) {}
    void discover_vertex(Vertex v, const Graph&) const {
        std::cout << "visit " << v << "\n";
        if (v == target) throw Found{};
    }
};

int main() {
    Graph g(5);
    add_edge(0, 1, g); add_edge(0, 2, g);
    add_edge(1, 3, g); add_edge(2, 4, g);

    try {
        breadth_first_search(g, 0, visitor(StopAt{3}));
    } catch (const Found&) {
        std::cout << "stopped at target\n";
    }
}
