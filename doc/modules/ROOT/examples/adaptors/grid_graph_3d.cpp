#include <boost/graph/grid_graph.hpp>
#include <boost/array.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = grid_graph<3>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    // 3x3x3 cube (no wrapping)
    boost::array<std::size_t, 3> lengths = {{ 3, 3, 3 }};
    Graph g(lengths);

    std::cout << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";

    // Corner vertex: 3 neighbors. Center vertex: 6 neighbors.
    Vertex corner = vertex(0, g);
    Vertex center = {{ 1, 1, 1 }};
    std::cout << "corner (" << corner[0] << "," << corner[1] << ","
              << corner[2] << ") degree=" << out_degree(corner, g) << "\n";
    std::cout << "center (" << center[0] << "," << center[1] << ","
              << center[2] << ") degree=" << out_degree(center, g) << "\n";
}
