#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <iostream>
#include <limits>
#include <vector>

struct Node {};
struct Edge { int weight; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, Node, Edge>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{4};
    add_edge(0, 1, Edge{1}, g);
    add_edge(1, 2, Edge{-2}, g);
    add_edge(0, 2, Edge{4}, g);
    add_edge(2, 3, Edge{3}, g);

    auto n = num_vertices(g);
    std::vector<int> dist(n, (std::numeric_limits<int>::max)());
    std::vector<Vertex> pred(n);
    dist[0] = 0;
    for (std::size_t i = 0; i < n; ++i)
        pred[i] = i;

    auto weight_map = get(&Edge::weight, g);
    auto dist_map = make_iterator_property_map(dist.begin(), get(vertex_index, g));
    auto pred_map = make_iterator_property_map(pred.begin(), get(vertex_index, g));

    bellman_ford_shortest_paths(g, n, weight_map, pred_map, dist_map,
        std::plus<int>(), std::less<int>(), default_bellman_visitor());

    for (auto v : make_iterator_range(vertices(g)))
        std::cout << "distance to " << v << " = " << dist[v] << "\n";
}
