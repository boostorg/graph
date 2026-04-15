#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <limits>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property,
                                 property<edge_weight_t, int>>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(4);
    add_edge(0, 1, 1, g);
    add_edge(0, 2, 4, g);
    add_edge(1, 2, 2, g);
    add_edge(1, 3, 6, g);
    add_edge(2, 3, 3, g);

    auto index = get(vertex_index, g);
    std::vector<int> dist(num_vertices(g));
    std::vector<Vertex> pred(num_vertices(g));
    auto dist_map = make_iterator_property_map(dist.begin(), index);
    auto pred_map = make_iterator_property_map(pred.begin(), index);

    dijkstra_shortest_paths(g, vertex(0, g),
        pred_map, dist_map, get(edge_weight, g), index,
        std::less<int>(), closed_plus<int>(),
        std::numeric_limits<int>::max(), 0,
        default_dijkstra_visitor());

    for (auto v : make_iterator_range(vertices(g))) {
        std::cout << "d[" << v << "] = " << dist[v] << "\n";
    }
}
