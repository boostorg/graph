#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <iostream>

// sloan_start_end_vertices requires property tags for color and degree.
using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS,
    property<vertex_color_t, default_color_type,
    property<vertex_degree_t, int,
    property<vertex_priority_t, double>>>>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(6);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 4, g);
    add_edge(4, 5, g); add_edge(0, 5, g);

    // The degree map must be initialized with actual vertex degrees.
    auto deg = get(vertex_degree, g);
    for (auto v : make_iterator_range(vertices(g))) {
        put(deg, v, static_cast<int>(out_degree(v, g)));
    }

    Vertex s = *vertices(g).first;
    Vertex e = sloan_start_end_vertices(g, s, get(vertex_color, g), deg);

    std::cout << "Start vertex: " << s << "\n";
    std::cout << "End vertex:   " << e << "\n";
}
