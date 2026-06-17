#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <limits>
#include <vector>

struct City {};
struct Road { int cost; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(4);
    add_edge(0, 1, Road{1}, g);
    add_edge(1, 2, Road{2}, g);
    add_edge(0, 2, Road{10}, g);
    add_edge(2, 3, Road{1}, g);

    // Storage: you control allocation, lifetime, and container type
    std::vector<Vertex> storage_pred(num_vertices(g));
    std::vector<int>    storage_dist(num_vertices(g));

    // Property maps: lightweight views into the storage
    auto index_map       = get(vertex_index, g);
    auto costs_map       = get(&Road::cost, g);
    auto predecessor_map = make_iterator_property_map(storage_pred.begin(), index_map);
    auto distance_map    = make_iterator_property_map(storage_dist.begin(), index_map);

    dijkstra_shortest_paths(g, vertex(0, g),
        predecessor_map, distance_map,
        costs_map, index_map,
        std::less<int>(), std::plus<int>(),
        std::numeric_limits<int>::max(), 0,
        dijkstra_visitor<null_visitor>());

    for (auto v : make_iterator_range(vertices(g)))
        std::cout << "distance to " << v << " = " << storage_dist[v] << "\n";
}
