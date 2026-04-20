#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/topology.hpp>
#include <iostream>
#include <vector>
#include <iomanip>

struct Spring { double length; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Spring>;

    Graph g(5);
    add_edge(0, 1, Spring{1.0}, g);
    add_edge(1, 2, Spring{1.0}, g);
    add_edge(2, 3, Spring{1.0}, g);
    add_edge(3, 4, Spring{1.0}, g);
    add_edge(4, 0, Spring{1.0}, g);

    using Topo = rectangle_topology<>;
    Topo topo(0.0, 0.0, 100.0, 100.0);
    using Point = Topo::point_type;

    std::vector<Point> pos(num_vertices(g));
    auto pos_map = make_iterator_property_map(pos.begin(), get(vertex_index, g));

    circle_graph_layout(g, pos_map, 25.0);

    // Bundled spring-length via member pointer — no edge_weight_t tag needed.
    bool ok = kamada_kawai_spring_layout(g, pos_map, get(&Spring::length, g),
        topo, side_length(50.0), layout_tolerance<double>(0.1));

    std::cout << std::fixed << std::setprecision(1);
    std::cout << "converged: " << (ok ? "yes" : "no") << "\n";
    for (auto v : make_iterator_range(vertices(g))) {
        std::cout << "vertex " << v << ": ("
                  << pos[v][0] << ", " << pos[v][1] << ")\n";
    }
}
