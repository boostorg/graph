#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, bidirectionalS>;

struct first_only_callback {
    bool found = false;
    template <typename Map1, typename Map2>
    bool operator()(Map1, Map2) { found = true; return false; }
};

int main() {
    // Small graph: triangle (0-1-2)
    Graph small(3);
    add_edge(0, 1, small); add_edge(1, 2, small); add_edge(2, 0, small);
    // Large graph: square with diagonal (0-1-2-3, plus 0-2)
    Graph large(4);
    add_edge(0, 1, large); add_edge(1, 2, large);
    add_edge(2, 3, large); add_edge(3, 0, large); add_edge(0, 2, large);

    first_only_callback cb;
    vf2_subgraph_iso(small, large, std::ref(cb));
    std::cout << "Subgraph isomorphism found: " << std::boolalpha
              << cb.found << "\n";
}
