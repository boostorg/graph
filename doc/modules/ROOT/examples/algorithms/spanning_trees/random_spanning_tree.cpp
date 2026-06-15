#include <iostream>
#include <vector>
#include <random>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_spanning_tree.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    Graph g(5);
    boost::add_edge(0, 1, g); boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g); boost::add_edge(1, 3, g);
    boost::add_edge(2, 3, g); boost::add_edge(3, 4, g);
    boost::add_edge(2, 4, g);

    std::vector<Vertex> pred(num_vertices(g));
    std::vector<boost::default_color_type> color(num_vertices(g));
    std::mt19937 gen(42); // fixed seed for deterministic output
    boost::random_spanning_tree(g, gen, Vertex{0},
        boost::make_iterator_property_map(pred.begin(), get(boost::vertex_index, g)),
        boost::static_property_map<double>(1.0),
        boost::make_iterator_property_map(color.begin(), get(boost::vertex_index, g)));

    std::cout << "Random spanning tree edges:\n";
    for (Vertex v = 0; v < num_vertices(g); ++v) {
        if (pred[v] != boost::graph_traits<Graph>::null_vertex() && pred[v] != v)
            std::cout << "  " << pred[v] << " - " << v << "\n";
    }
}
