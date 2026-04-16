#include <boost/graph/grid_graph.hpp>
#include <boost/array.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = grid_graph<2>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    // 4x3 cylinder: dimension 0 wraps, dimension 1 does not
    boost::array<std::size_t, 2> lengths = {{ 4, 3 }};
    boost::array<bool, 2> wrap = {{ true, false }};
    Graph g(lengths, wrap);

    std::cout << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";

    // Navigate: wrapping in dimension 0
    Vertex corner = vertex(0, g);
    Vertex wrapped = g.previous(corner, 0);  // wraps to (3,0)
    std::cout << "\nFrom (" << corner[0] << "," << corner[1] << "):\n";
    std::cout << "  previous in dim 0 (wraps) = ("
              << wrapped[0] << "," << wrapped[1] << ")\n";

    // Navigate: no wrapping in dimension 1
    Vertex same = g.previous(corner, 1);  // stays at (0,0)
    std::cout << "  previous in dim 1 (stops) = ("
              << same[0] << "," << same[1] << ")\n";
}
