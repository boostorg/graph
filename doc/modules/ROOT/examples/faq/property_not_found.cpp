#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <iostream>
#include <vector>

using namespace boost;
struct Edge { int weight; };
using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;

int main() {
    Graph g(3);
    add_edge(0, 1, Edge{5}, g);
    add_edge(1, 2, Edge{3}, g);
    add_edge(0, 2, Edge{9}, g);

    std::vector<int> d(num_vertices(g));
    // Graph has no edge_weight_t tag; pass the bundled field as the weight map.
    dijkstra_shortest_paths(g, 0,
        weight_map(get(&Edge::weight, g))
            .distance_map(make_iterator_property_map(
                d.begin(), get(vertex_index, g))));

    std::cout << "shortest distance 0 -> 2 = " << d[2] << "\n";
}
