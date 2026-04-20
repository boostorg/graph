// Demonstrates the "mutually recursive type" problem that
// adjacency_list_traits solves.
//
// We want an interior edge property whose value is a vertex_descriptor
// (for example, a pointer-to-predecessor stored on each edge). That
// requires the vertex_descriptor type to be known BEFORE we finish
// declaring the graph — which is impossible with graph_traits because
// graph_traits needs the full graph type.
//
// adjacency_list_traits<EdgeList, VertexList, Directed> exposes just
// vertex_descriptor / edge_descriptor without instantiating the full
// adjacency_list template, breaking the cycle.

#include <boost/graph/adjacency_list.hpp>
#include <iostream>

using namespace boost;

// Step 1: pre-declare the traits. Same selectors as the final graph.
using Traits = adjacency_list_traits<vecS, vecS, directedS>;

// Step 2: use Traits::vertex_descriptor as a property value type.
using Graph = adjacency_list<
    vecS, vecS, directedS,
    no_property,
    property<edge_name_t, Traits::vertex_descriptor>>;

int main() {
    Graph g(3);
    auto e01 = add_edge(0, 1, g).first;
    auto e12 = add_edge(1, 2, g).first;

    // Store the predecessor vertex on each edge.
    put(edge_name, g, e01, vertex(0, g));
    put(edge_name, g, e12, vertex(1, g));

    for (auto e : make_iterator_range(edges(g))) {
        std::cout << source(e, g) << " -> " << target(e, g)
                  << "  predecessor=" << get(edge_name, g, e) << '\n';
    }
}
