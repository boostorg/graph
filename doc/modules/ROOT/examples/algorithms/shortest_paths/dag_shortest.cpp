#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dag_shortest_paths.hpp>
#include <iostream>
#include <limits>
#include <vector>

struct Node {};
struct Edge { int weight; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, Node, Edge>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{5};
    add_edge(0, 1, Edge{2}, g);
    add_edge(0, 2, Edge{6}, g);
    add_edge(1, 3, Edge{5}, g);
    add_edge(2, 3, Edge{1}, g);
    add_edge(3, 4, Edge{3}, g);

    std::vector<int> dist(num_vertices(g));
    std::vector<Vertex> pred(num_vertices(g));
    std::vector<default_color_type> color(num_vertices(g));

    auto idx = get(vertex_index, g);
    dag_shortest_paths(g, vertex(0, g),
        make_iterator_property_map(dist.begin(), idx),
        get(&Edge::weight, g),
        make_iterator_property_map(color.begin(), idx),
        make_iterator_property_map(pred.begin(), idx),
        dijkstra_visitor<null_visitor>(),
        std::less<int>(), std::plus<int>(),
        (std::numeric_limits<int>::max)(), 0);

    for (auto v : make_iterator_range(vertices(g)))
        std::cout << "distance to " << v << " = " << dist[v] << "\n";
}
