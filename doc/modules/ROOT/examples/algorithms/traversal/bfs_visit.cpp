#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/pending/queue.hpp>
#include <iostream>
#include <vector>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

struct PrintVisitor : boost::default_bfs_visitor {
    void discover_vertex(Vertex v, const Graph& g) const {
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

    // breadth_first_visit skips initialization (no white-painting of all vertices).
    // Caller must provide a color map and queue.
    std::vector<boost::default_color_type> colors(num_vertices(g), boost::white_color);
    auto color_map = boost::make_iterator_property_map(colors.begin(), get(boost::vertex_index, g));
    boost::queue<Vertex> q;

    std::cout << "BFS visit order: ";
    boost::breadth_first_visit(g, vertex(0, g), q, PrintVisitor{}, color_map);
    std::cout << std::endl;
}
