#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <iostream>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;

struct DFSVisitor : boost::default_dfs_visitor {
    void discover_vertex(Graph::vertex_descriptor v, const Graph& g) const {
        std::cout << "discover " << g[v].id << "\n";
    }
    void finish_vertex(Graph::vertex_descriptor v, const Graph& g) const {
        std::cout << "finish   " << g[v].id << "\n";
    }
};

int main() {
    Graph g{4};
    for (int i = 0; i < 4; ++i) { g[i].id = i; }
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);

    boost::depth_first_search(g, boost::visitor(DFSVisitor{}));
}
