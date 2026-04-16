#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>
#include <iostream>
#include <sstream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS,
        property<vertex_name_t, std::string>,
        property<edge_weight_t, int>>;

    // --- Write ---
    Graph g(3);
    add_edge(0, 1, {10}, g);
    add_edge(1, 2, {20}, g);
    add_edge(0, 2, {30}, g);
    put(vertex_name, g, 0, "Paris");
    put(vertex_name, g, 1, "Lyon");
    put(vertex_name, g, 2, "Marseille");

    dynamic_properties dp;
    dp.property("name", get(vertex_name, g));
    dp.property("weight", get(edge_weight, g));

    std::ostringstream xml;
    write_graphml(xml, g, dp, true);
    std::cout << "=== GraphML output ===\n" << xml.str() << "\n";

    // --- Read back ---
    Graph g2;
    dynamic_properties dp2;
    dp2.property("name", get(vertex_name, g2));
    dp2.property("weight", get(edge_weight, g2));

    std::istringstream in(xml.str());
    read_graphml(in, g2, dp2);

    std::cout << "=== Read back ===\n";
    std::cout << num_vertices(g2) << " vertices, "
              << num_edges(g2) << " edges\n";
    for (auto v : make_iterator_range(vertices(g2))) {
        std::cout << "  " << get(vertex_name, g2, v) << "\n";
    }
}
