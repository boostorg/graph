// The inverse adjacency iterator iterates SOURCE vertices of in-edges.
//
// Most BGL users never construct one directly — they call
// `inv_adjacent_vertices(v, g)` on a BidirectionalGraph. You only need
// `inv_adjacency_iterator_generator` when implementing a new graph
// type that models BidirectionalGraph and wants to synthesize its
// `inv_adjacency_iterator` from its `in_edge_iterator`.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, bidirectionalS>;

int main() {
    Graph g(4);
    add_edge(1, 0, g);
    add_edge(2, 0, g);
    add_edge(3, 0, g);

    // Everyday path: inv_adjacent_vertices returns an (iterator, iterator)
    // pair whose value_type is vertex_descriptor.
    std::cout << "predecessors of 0:";
    for (auto v : make_iterator_range(inv_adjacent_vertices(0, g))) {
        std::cout << ' ' << v;
    }
    std::cout << '\n';
}
