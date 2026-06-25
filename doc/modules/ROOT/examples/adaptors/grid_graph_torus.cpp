#include <boost/graph/grid_graph.hpp>
#include <boost/array.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = grid_graph<2>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    // 3x3 torus (all dimensions wrap)
    boost::array<std::size_t, 2> lengths = {{ 3, 3 }};
    Graph g(lengths, true);

    std::cout << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";

    // Every vertex has exactly 4 neighbors on a torus
    for (auto vi = vertices(g).first; vi != vertices(g).second; ++vi) {
        Vertex v = *vi;
        std::cout << "(" << v[0] << "," << v[1] << ") degree="
                  << out_degree(v, g) << "\n";
    }
}
