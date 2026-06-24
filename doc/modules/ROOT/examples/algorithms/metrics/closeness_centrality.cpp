#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/closeness_centrality.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>

// Bundled edge property holding the weight
struct EdgeProps {
    int weight;
};

int main() {
    // Undirected graph with bundled edge properties
    using Graph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::undirectedS,
        boost::no_property, EdgeProps>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

    Graph g{5};

    // Build a small weighted undirected graph:
    //   0 --1-- 1 --2-- 2
    //   |               |
    //   3               1
    //   |               |
    //   3 ------2------ 4
    add_edge(0, 1, EdgeProps{1}, g);
    add_edge(1, 2, EdgeProps{2}, g);
    add_edge(0, 3, EdgeProps{3}, g);
    add_edge(2, 4, EdgeProps{1}, g);
    add_edge(3, 4, EdgeProps{2}, g);

    auto weight_map = boost::get(&EdgeProps::weight, g);
    auto index_map = boost::get(boost::vertex_index, g);

    std::cout << std::fixed << std::setprecision(4);
    std::cout << "Closeness centrality (1 / sum of shortest distances):\n";

    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        // Run Dijkstra from vertex v to get shortest distances
        auto n = boost::num_vertices(g);
        std::vector<int> distances(n);
        std::vector<Vertex> predecessors(n);
        auto dist_map = boost::make_iterator_property_map(
            distances.begin(), index_map);
        auto pred_map = boost::make_iterator_property_map(
            predecessors.begin(), index_map);

        boost::dijkstra_shortest_paths(g, v,
            pred_map, dist_map, weight_map, index_map,
            std::less<int>{}, std::plus<int>{},
            (std::numeric_limits<int>::max)(), int{0},
            boost::default_dijkstra_visitor{});

        // Compute closeness centrality from the distance map
        double cc = boost::closeness_centrality(g, dist_map);
        std::cout << "  vertex " << v << ": " << cc << "\n";
    }
}
