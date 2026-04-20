#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <iostream>
#include <vector>

using namespace boost;

struct VertexData {
    default_color_type color{};
    int degree = 0;
    double priority = 0.0;
};

using Graph = adjacency_list<setS, vecS, undirectedS, VertexData>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(6);
    add_edge(0, 1, g); add_edge(0, 2, g); add_edge(1, 3, g);
    add_edge(2, 3, g); add_edge(3, 4, g); add_edge(4, 5, g);

    // Populate degrees; sloan_ordering reads them to pick a start vertex.
    auto deg = get(&VertexData::degree, g);
    for (auto v : make_iterator_range(vertices(g))) {
        put(deg, v, static_cast<int>(out_degree(v, g)));
    }

    // Bundled scratch fields replace the three vertex property tags.
    std::vector<Vertex> order(num_vertices(g));
    sloan_ordering(g, order.begin(),
        get(&VertexData::color, g), deg,
        get(&VertexData::priority, g));

    std::cout << "Sloan ordering:";
    for (auto v : order) { std::cout << " " << v; }
    std::cout << "\n";
}
