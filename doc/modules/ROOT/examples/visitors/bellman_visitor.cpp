#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>
#include <limits>

struct Road { double km; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property, Road>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(4);
    add_edge(0, 1, Road{1.0}, g);
    add_edge(1, 2, Road{-2.0}, g);
    add_edge(0, 2, Road{4.0}, g);
    add_edge(2, 3, Road{3.0}, g);

    std::vector<Vertex> pred(num_vertices(g));
    std::vector<double> dist(num_vertices(g),
                             (std::numeric_limits<double>::max)());
    dist[0] = 0.0;
    auto index = get(vertex_index, g);
    auto pred_map = make_iterator_property_map(pred.begin(), index);
    auto dist_map = make_iterator_property_map(dist.begin(), index);

    auto vis = make_bellman_visitor(
        record_predecessors(pred_map, on_edge_relaxed())
    );

    bellman_ford_shortest_paths(g, num_vertices(g),
        get(&Road::km, g), pred_map, dist_map,
        std::plus<double>(), std::less<double>(), vis);

    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v
                  << "  dist=" << dist[v]
                  << "  pred=" << pred[v] << "\n";
    }
}
