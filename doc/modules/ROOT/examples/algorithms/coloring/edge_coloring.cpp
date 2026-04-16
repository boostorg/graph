#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_coloring.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, boost::property<boost::edge_color_t, int>>;
    Graph g(4);
    boost::add_edge(0, 1, g); boost::add_edge(0, 2, g);
    boost::add_edge(0, 3, g); boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);

    auto color_map = get(boost::edge_color, g);
    auto num_colors = boost::edge_coloring(g, color_map);

    std::cout << "Edge coloring uses " << num_colors << " colors:\n";
    for (auto ep = edges(g); ep.first != ep.second; ++ep.first) {
        auto e = *ep.first;
        std::cout << "  " << source(e, g) << "-" << target(e, g)
                  << " : color " << get(color_map, e) << "\n";
    }
}
