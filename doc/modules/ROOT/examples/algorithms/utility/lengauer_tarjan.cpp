#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dominator_tree.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    Graph g(6);
    // CFG-like: 0 is entry
    boost::add_edge(0, 1, g); boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g); boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g); boost::add_edge(4, 5, g);

    std::vector<Vertex> dom(num_vertices(g), boost::graph_traits<Graph>::null_vertex());
    auto dom_map = boost::make_iterator_property_map(dom.begin(), get(boost::vertex_index, g));
    boost::lengauer_tarjan_dominator_tree(g, Vertex{0}, dom_map);

    std::cout << "Immediate dominators (entry = 0):\n";
    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        if (dom[v] != boost::graph_traits<Graph>::null_vertex())
            std::cout << "  idom(" << v << ") = " << dom[v] << "\n";
        else
            std::cout << "  idom(" << v << ") = none (entry)\n";
    }
}
