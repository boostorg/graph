#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random_layout.hpp>
#include <boost/graph/topology.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Point = rectangle_topology<>::point_type;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

int main() {
    Graph g(5);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 4, g);

    minstd_rand gen(42);
    rectangle_topology<> topology(gen, 0.0, 0.0, 100.0, 100.0);

    std::vector<Point> positions(num_vertices(g));
    auto pos_map = make_iterator_property_map(
        positions.begin(), get(vertex_index, g));

    random_graph_layout(g, pos_map, topology);

    for (auto v : make_iterator_range(vertices(g)))
        std::cout << "vertex " << v << ": ("
                  << positions[v][0] << ", " << positions[v][1] << ")\n";
}
