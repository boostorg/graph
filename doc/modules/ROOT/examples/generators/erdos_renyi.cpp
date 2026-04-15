#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using EdgeCount = graph_traits<Graph>::edges_size_type;
    using ERGen = erdos_renyi_iterator<minstd_rand, Graph>;

    minstd_rand gen(42);
    Graph g(ERGen(gen, 6, EdgeCount(8)), ERGen(), 6);

    std::cout << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
