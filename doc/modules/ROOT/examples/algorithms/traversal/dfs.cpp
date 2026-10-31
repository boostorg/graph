#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;

// records the events in the order the search reaches them
struct DFSVisitor : boost::default_dfs_visitor {
    std::vector<std::string> events;
    void discover_vertex(Graph::vertex_descriptor v, const Graph& g) {
        events.push_back("discover " + std::to_string(g[v].id));
    }
    void finish_vertex(Graph::vertex_descriptor v, const Graph& g) {
        events.push_back("finish   " + std::to_string(g[v].id));
    }
};

int main() {
    Graph g{4};
    for (int i = 0; i < 4; ++i) { g[i].id = i; }
    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);

    DFSVisitor visitor;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::depth_first_search(g, boost::visitor(std::ref(visitor)));

    for (const std::string& event : visitor.events) std::cout << event << "\n";
}
