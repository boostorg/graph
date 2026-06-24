#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/ssca_graph_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using SSCAGen = ssca_iterator<mt19937, Graph>;

    mt19937 gen(42);

    int max_clique_size = 3;
    double prob_unidirectional = 0.5;
    int max_parallel_edges = 1;
    double prob_interclique = 0.1;
    Graph g(SSCAGen(gen, 8, max_clique_size, prob_unidirectional, max_parallel_edges, prob_interclique), SSCAGen(), 8);

    std::cout << num_vertices(g) << " vertices, " << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
