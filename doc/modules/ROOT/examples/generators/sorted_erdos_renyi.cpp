#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using SERGen = sorted_erdos_renyi_iterator<minstd_rand, Graph>;

    minstd_rand gen(42);
    double prob = 0.3;
    Graph g(SERGen(gen, 6, prob), SERGen(), 6);

    std::cout << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
