// Sequential Vertex Coloring example
#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sequential_vertex_coloring.hpp>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{5};
    boost::add_edge(0, 1, g); boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g); boost::add_edge(1, 3, g);
    boost::add_edge(2, 4, g); boost::add_edge(3, 4, g);

    std::vector<int> colors(boost::num_vertices(g));
    auto color_map = boost::make_iterator_property_map(
        colors.begin(), boost::get(boost::vertex_index, g));
    int num_colors = boost::sequential_vertex_coloring(g, color_map);

    std::cout << "Colors used: " << num_colors << "\n";
    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        std::cout << "  Vertex " << v << " -> color " << colors[v] << "\n";
    }
}
