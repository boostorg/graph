#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/fruchterman_reingold.hpp>
#include <boost/graph/topology.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;
using Point = rectangle_topology<>::point_type;

int main() {
    Graph g(5);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 4, g); add_edge(4, 0, g);

    minstd_rand gen(42);
    rectangle_topology<> topology(gen, 0.0, 0.0, 100.0, 100.0);

    std::vector<Point> positions(num_vertices(g));
    for (auto& p : positions) { p = topology.random_point(); }
    auto pos_map = make_iterator_property_map(positions.begin(), get(vertex_index, g));

    fruchterman_reingold_force_directed_layout(g, pos_map, topology);

    for (auto v : make_iterator_range(vertices(g))) {
        auto& p = positions[v];
        std::cout << "vertex " << v << ": (" << p[0] << ", " << p[1] << ")\n";
    }
}
