#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/undirected_dfs.hpp>
#include <functional>
#include <iostream>
#include <string>
#include <vector>

struct VertexProps { int id; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, VertexProps>;
using Edge = boost::graph_traits<Graph>::edge_descriptor;

// records the events in the order the search reaches them
struct Visitor : boost::default_dfs_visitor {
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

    using ColorMap = std::map<Graph::vertex_descriptor, boost::default_color_type>;
    using EdgeColorMap = std::map<Edge, boost::default_color_type>;
    ColorMap vcmap;
    EdgeColorMap ecmap;
    Visitor visitor;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::undirected_dfs(g, std::ref(visitor),
        boost::make_assoc_property_map(vcmap),
        boost::make_assoc_property_map(ecmap));

    for (const std::string& event : visitor.events) std::cout << event << "\n";
}
