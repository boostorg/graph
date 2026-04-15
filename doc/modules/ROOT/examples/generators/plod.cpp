#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/plod_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using PLODGen = plod_iterator<minstd_rand, Graph>;

    minstd_rand gen(42);

    double alpha = 2.5;
    double beta = 10;
    Graph g(PLODGen(gen, 8, alpha, beta), PLODGen(), 8);

    std::cout << num_vertices(g) << " vertices, " << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
