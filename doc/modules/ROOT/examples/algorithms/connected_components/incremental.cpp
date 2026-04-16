#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/pending/disjoint_sets.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    Graph g(5);
    std::vector<std::size_t> rank(5, 0);
    std::vector<std::size_t> parent(5);
    boost::disjoint_sets<std::size_t*, std::size_t*> ds(rank.data(), parent.data());
    for (std::size_t i = 0; i < 5; ++i) ds.make_set(i);

    auto add_and_report = [&](int u, int v) {
        boost::add_edge(u, v, g);
        ds.union_set(static_cast<std::size_t>(u), static_cast<std::size_t>(v));
        // Count components
        std::size_t count = 0;
        for (std::size_t i = 0; i < 5; ++i)
            if (ds.find_set(i) == i) ++count;
        std::cout << "  After adding " << u << "-" << v << ": " << count << " components\n";
    };

    std::cout << "Incremental connected components:\n";
    add_and_report(0, 1);
    add_and_report(2, 3);
    add_and_report(1, 2);
    add_and_report(3, 4);
}
