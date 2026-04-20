// Most BGL users never construct an adjacency_iterator directly —
// they call `adjacent_vertices(v, g)`, which returns a pair of them.
//
// This example shows the plain-vanilla use via adjacent_vertices, then
// the `type` exposed by adjacency_iterator_generator for the same graph.
// You only reach for the generator when implementing a new graph type
// that models IncidenceGraph but not (yet) AdjacencyGraph.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <iostream>
#include <type_traits>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS>;

int main() {
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 3, g);

    // Everyday path: adjacent_vertices returns an (iterator, iterator) pair
    // whose value_type is vertex_descriptor.
    std::cout << "neighbors of 0:";
    for (auto v : make_iterator_range(adjacent_vertices(0, g))) {
        std::cout << ' ' << v;
    }
    std::cout << '\n';

    // The generator produces a type equivalent to what adjacency_list
    // already exposes as graph_traits<Graph>::adjacency_iterator.
    using Generated = adjacency_iterator_generator<
        Graph,
        graph_traits<Graph>::vertex_descriptor,
        graph_traits<Graph>::out_edge_iterator>::type;

    using Existing = graph_traits<Graph>::adjacency_iterator;

    std::cout << "generator matches graph_traits: "
              << std::boolalpha << std::is_same<Generated, Existing>::value
              << '\n';
}
