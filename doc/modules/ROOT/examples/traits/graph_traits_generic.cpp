#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <iostream>

// Generic function: Graph is a template parameter, so every nested type
// must be disambiguated with `typename graph_traits<Graph>::...`.
template <typename Graph>
typename boost::graph_traits<Graph>::degree_size_type
sum_out_degrees(const Graph& g) {
    using Traits = boost::graph_traits<Graph>;
    typename Traits::degree_size_type total = 0;
    for (auto v : boost::make_iterator_range(vertices(g))) {
        total += out_degree(v, g);
    }
    return total;
}

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;
    Graph g(3);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(0, 2, g);

    std::cout << "sum of out-degrees = " << sum_out_degrees(g) << '\n';
}
