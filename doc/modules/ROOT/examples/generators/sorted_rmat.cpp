#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/rmat_graph_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using EdgeCount = graph_traits<Graph>::edges_size_type;
    using SGen = sorted_rmat_iterator<mt19937, Graph>;

    mt19937 gen(42);
    double a = 0.57, b = 0.19, c = 0.19, d = 0.05;
    Graph g(SGen(gen, 8, EdgeCount(10), a, b, c, d), SGen(), 8);

    std::cout << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
