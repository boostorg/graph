#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/topology.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Point = convex_topology<2>::point_type;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

int main() {
    Graph g(5);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 4, g); add_edge(4, 0, g);

    std::vector<Point> positions(num_vertices(g));
    auto pos_map = make_iterator_property_map(
        positions.begin(), get(vertex_index, g));

    circle_graph_layout(g, pos_map, 50.0);

    for (auto v : make_iterator_range(vertices(g)))
        std::cout << "vertex " << v << ": ("
                  << positions[v][0] << ", " << positions[v][1] << ")\n";
}
