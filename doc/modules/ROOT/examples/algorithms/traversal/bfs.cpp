#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>
#include <vector>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;

struct DiscoverVisitor : boost::default_bfs_visitor {
    void discover_vertex(Graph::vertex_descriptor v, const Graph& g) const {
        std::cout << g[v].id << " ";
    }
};

int main() {
    Graph g{5};
    for (int i = 0; i < 5; ++i) { g[i].id = i; }
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);
    boost::add_edge(2, 4, g);

    std::cout << "BFS discovery order: ";
    boost::breadth_first_search(g, 0, boost::visitor(DiscoverVisitor{}));
    std::cout << std::endl;
}
