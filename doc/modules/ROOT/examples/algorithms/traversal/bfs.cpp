#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <functional>
#include <iostream>
#include <vector>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;

// records the vertices in the order the search discovers them
struct DiscoverRecorder : boost::default_bfs_visitor {
    std::vector<int> order;
    void discover_vertex(Graph::vertex_descriptor v, const Graph& g) {
        order.push_back(g[v].id);
    }
};

int main() {
    Graph g{5};
    for (int i = 0; i < 5; ++i) { g[i].id = i; }
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);
    boost::add_edge(2, 4, g);

    DiscoverRecorder visitor;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::breadth_first_search(g, 0, boost::visitor(std::ref(visitor)));

    std::cout << "BFS discovery order: ";
    for (int id : visitor.order) std::cout << id << " ";
    std::cout << std::endl;
}
