#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <map>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, listS, directedS>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g;
    Vertex a = add_vertex(g), b = add_vertex(g);
    Vertex c = add_vertex(g), d = add_vertex(g);
    add_edge(a, b, g);
    add_edge(a, c, g);
    add_edge(b, d, g);
    add_edge(c, d, g);

    std::map<Vertex, int> depth;
    auto depth_map = make_assoc_property_map(depth);

    put(depth_map, a, 0);
    put(depth_map, b, 1);
    put(depth_map, c, 1);
    put(depth_map, d, 2);

    for (auto v : make_iterator_range(vertices(g))) {
        std::cout << "depth = " << get(depth_map, v) << "\n";
    }
}
