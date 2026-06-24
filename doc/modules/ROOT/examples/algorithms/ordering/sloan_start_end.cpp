#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <iostream>

using namespace boost;

struct VertexData {
    default_color_type color;
    int degree;
};

using Graph = adjacency_list<vecS, vecS, undirectedS, VertexData>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(6);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 4, g);
    add_edge(4, 5, g); add_edge(0, 5, g);

    // sloan_start_end_vertices reads the degree field, so populate it first.
    auto deg = get(&VertexData::degree, g);
    for (auto v : make_iterator_range(vertices(g))) {
        put(deg, v, static_cast<int>(out_degree(v, g)));
    }

    Vertex s = *vertices(g).first;
    Vertex e = sloan_start_end_vertices(g, s,
        get(&VertexData::color, g), deg);

    std::cout << "Start vertex: " << s << "\n";
    std::cout << "End vertex:   " << e << "\n";
}
