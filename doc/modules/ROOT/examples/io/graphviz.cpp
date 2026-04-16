#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <sstream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS,
        property<vertex_name_t, std::string>,
        property<edge_weight_t, double>>;

    // --- Write ---
    Graph g(3);
    add_edge(0, 1, {1.5}, g);
    add_edge(1, 2, {2.5}, g);
    add_edge(0, 2, {4.0}, g);
    put(vertex_name, g, 0, "A");
    put(vertex_name, g, 1, "B");
    put(vertex_name, g, 2, "C");

    dynamic_properties dp;
    dp.property("node_id", get(vertex_name, g));
    dp.property("weight", get(edge_weight, g));

    std::ostringstream dot;
    write_graphviz_dp(dot, g, dp, "node_id");
    std::cout << "=== DOT output ===\n" << dot.str();

    // --- Read back ---
    Graph g2;
    dynamic_properties dp2;
    dp2.property("node_id", get(vertex_name, g2));
    dp2.property("weight", get(edge_weight, g2));

    std::istringstream in(dot.str());
    read_graphviz(in, g2, dp2, "node_id");

    std::cout << "=== Read back ===\n";
    std::cout << num_vertices(g2) << " vertices, "
              << num_edges(g2) << " edges\n";
}
