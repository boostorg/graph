#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/small_world_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS>;
    using SWGen = small_world_iterator<minstd_rand, Graph>;

    minstd_rand gen(42);

    int k = 2;
    double rewire_prob = 0.1;
    Graph g(SWGen(gen, 6, k, rewire_prob), SWGen(), 6);

    std::cout << num_vertices(g) << " vertices, " << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -- " << target(e, g) << "\n";
    }
}
