#include <boost/graph/grid_graph.hpp>
#include <boost/array.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = grid_graph<2>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    // 3x3 flat grid (no wrapping)
    boost::array<std::size_t, 2> lengths = {{ 3, 3 }};
    Graph g(lengths);

    std::cout << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";

    // Corner vertex has 2 neighbors, edge vertex has 3, center has 4
    for (auto vi = vertices(g).first; vi != vertices(g).second; ++vi) {
        Vertex v = *vi;
        std::cout << "(" << v[0] << "," << v[1] << ") degree="
                  << out_degree(v, g) << "\n";
    }
}
