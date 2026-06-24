#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <vector>
#include <limits>

struct Road { double km; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property, Road>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(5);
    add_edge(0, 1, Road{1.0}, g);
    add_edge(0, 2, Road{4.0}, g);
    add_edge(1, 2, Road{2.0}, g);
    add_edge(1, 3, Road{6.0}, g);
    add_edge(2, 3, Road{3.0}, g);
    add_edge(3, 4, Road{1.0}, g);

    std::vector<Vertex> pred(num_vertices(g));
    std::vector<double> dist(num_vertices(g));
    auto weight = get(&Road::km, g);
    auto index = get(vertex_index, g);
    auto pred_map = make_iterator_property_map(pred.begin(), index);
    auto dist_map = make_iterator_property_map(dist.begin(), index);

    auto vis = make_dijkstra_visitor(
        record_predecessors(pred_map, on_edge_relaxed())
    );

    dijkstra_shortest_paths(g, vertex(0, g),
        pred_map, dist_map, weight, index,
        std::less<double>(), std::plus<double>(),
        (std::numeric_limits<double>::max)(), 0.0,
        vis);

    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v
                  << "  dist=" << dist[v]
                  << "  pred=" << pred[v] << "\n";
    }
}
