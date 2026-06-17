#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/gursoy_atun_layout.hpp>
#include <boost/graph/topology.hpp>
#include <iostream>
#include <vector>

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Point = boost::heart_topology<>::point_type;

int main() {
    Graph g(5);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(2, 3, g);
    boost::add_edge(3, 4, g);
    boost::add_edge(4, 0, g);

    std::vector<Point> positions(num_vertices(g));
    auto pos_map = boost::make_iterator_property_map(
        positions.begin(), get(boost::vertex_index, g));

    boost::heart_topology<> topo;
    boost::gursoy_atun_layout(g, topo, pos_map);

    for (auto v : boost::make_iterator_range(vertices(g))) {
        auto& p = positions[v];
        std::cout << "vertex " << v << ": (" << p[0] << ", " << p[1] << ")\n";
    }
}
