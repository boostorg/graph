#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/max_cardinality_matching.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    Graph g(6);
    boost::add_edge(0, 1, g); boost::add_edge(0, 3, g);
    boost::add_edge(1, 2, g); boost::add_edge(2, 3, g);
    boost::add_edge(4, 5, g);

    std::vector<Vertex> mate(num_vertices(g));
    auto mate_map = boost::make_iterator_property_map(
        mate.begin(), get(boost::vertex_index, g));
    boost::edmonds_maximum_cardinality_matching(g, mate_map);

    std::cout << "Maximum matching (" << boost::matching_size(g, mate_map,
        get(boost::vertex_index, g)) << " edges):\n";
    for (Vertex v = 0; v < num_vertices(g); ++v) {
        if (mate[v] != boost::graph_traits<Graph>::null_vertex() && v < mate[v])
            std::cout << "  " << v << " - " << mate[v] << "\n";
    }
}
