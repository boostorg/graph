#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <limits>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property,
                                 property<edge_weight_t, int>>;

    Graph g(4);
    add_edge(0, 1, 1, g);
    add_edge(0, 2, 4, g);
    add_edge(1, 2, 2, g);
    add_edge(1, 3, 6, g);
    add_edge(2, 3, 3, g);

    int dist[4];
    int pred[4];

    dijkstra_shortest_paths(g, vertex(0, g),
        pred, dist, get(edge_weight, g), get(vertex_index, g),
        std::less<int>(), closed_plus<int>(),
        std::numeric_limits<int>::max(), 0,
        default_dijkstra_visitor());

    for (int i = 0; i < 4; ++i) {
        std::cout << "d[" << i << "] = " << dist[i] << "\n";
    }
}
