#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_weight_t, double>>;

int main() {
    Graph g;
    mt19937 gen(42);

    bool allow_parallel = true;
    bool allow_self_loops = false;
    generate_random_graph(g, 5, 8, gen, allow_parallel, allow_self_loops);
    std::cout << num_vertices(g) << " vertices, " << num_edges(g) << " edges\n";

    // Assign random weights to all edges
    randomize_property<edge_weight_t>(g, gen);

    std::cout << "random vertex: " << random_vertex(g, gen) << "\n";

    auto e = random_edge(g, gen);
    std::cout << "random edge: " << source(e, g) << " -> " << target(e, g) << "\n";

    if (out_degree(vertex(0, g), g) > 0) {
        auto oe = random_out_edge(g, vertex(0, g), gen);
        std::cout << "random out-edge of 0: "
                  << source(oe, g) << " -> " << target(oe, g) << "\n";

        auto we = weighted_random_out_edge(g, vertex(0, g), get(edge_weight, g), gen);
        std::cout << "weighted random out-edge of 0: "
                  << source(we, g) << " -> " << target(we, g) << "\n";
    }
}
