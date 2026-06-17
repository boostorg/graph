#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/read_dimacs.hpp>
#include <boost/graph/write_dimacs.hpp>
#include <iostream>
#include <sstream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS,
        no_property,
        property<edge_capacity_t, long,
            property<edge_residual_capacity_t, long,
                property<edge_reverse_t,
                    graph_traits<adjacency_list<vecS, vecS, directedS,
                        no_property,
                        property<edge_capacity_t, long,
                            property<edge_residual_capacity_t, long,
                                property<edge_reverse_t, void*>>>>>::edge_descriptor>>>>;

    // DIMACS max-flow input
    std::string dimacs_input =
        "c This is a comment\n"
        "p max 4 5\n"
        "n 1 s\n"
        "n 4 t\n"
        "a 1 2 10\n"
        "a 1 3 20\n"
        "a 2 3 5\n"
        "a 2 4 15\n"
        "a 3 4 10\n";

    // --- Read ---
    Graph g;
    auto capacity = get(edge_capacity, g);
    auto reverse_edge = get(edge_reverse, g);
    graph_traits<Graph>::vertex_descriptor src, sink;

    std::istringstream in(dimacs_input);
    read_dimacs_max_flow(g, capacity, reverse_edge, src, sink, in);

    std::cout << num_vertices(g) << " vertices, "
              << num_edges(g) << " edges\n";
    std::cout << "source=" << src << " sink=" << sink << "\n";

    // --- Write ---
    std::cout << "\n=== DIMACS output ===\n";
    write_dimacs_max_flow(g, capacity, get(vertex_index, g),
                          src, sink, std::cout);
}
