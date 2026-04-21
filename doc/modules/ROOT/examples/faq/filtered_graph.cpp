#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <iostream>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS>;

struct KeepEven {
    const Graph* g = nullptr;
    bool operator()(graph_traits<Graph>::vertex_descriptor v) const {
        return v % 2 == 0;
    }
    bool operator()(graph_traits<Graph>::edge_descriptor e) const {
        return (*this)(source(e, *g)) && (*this)(target(e, *g));
    }
};

int main() {
    Graph g(6);
    for (int u = 0; u < 6; ++u)
        for (int v = u + 1; v < 6; ++v)
            add_edge(u, v, g);  // K6

    KeepEven p{&g};
    filtered_graph<Graph, KeepEven, KeepEven> sub(g, p, p);

    std::cout << "filtered vertices:";
    for (auto v : make_iterator_range(vertices(sub))) std::cout << " " << v;

    // num_edges on a filtered_graph returns the underlying count; count manually.
    int kept = 0;
    for (auto e : make_iterator_range(edges(sub))) { (void)e; ++kept; }
    std::cout << "\nfiltered edges: " << kept << "\n";
}
