#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/betweenness_centrality.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    Graph g{5};
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g);

    std::vector<double> centrality(num_vertices(g), 0.0);
    boost::brandes_betweenness_centrality(g,
        boost::make_iterator_property_map(centrality.begin(),
            get(boost::vertex_index, g)));

    std::cout << "Betweenness centrality:\n";
    for (std::size_t i = 0; i < centrality.size(); ++i) {
        std::cout << "  Vertex " << i << ": " << centrality[i] << "\n";
    }
}
