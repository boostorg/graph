#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <iostream>
#include <sstream>

struct City { std::string name; };
struct Road { double weight; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;

    // --- Write ---
    Graph g(3);
    g[0].name = "A";
    g[1].name = "B";
    g[2].name = "C";
    add_edge(0, 1, Road{1.5}, g);
    add_edge(1, 2, Road{2.5}, g);
    add_edge(0, 2, Road{4.0}, g);

    dynamic_properties dp;
    dp.property("node_id", get(&City::name, g));
    dp.property("weight", get(&Road::weight, g));

    std::ostringstream dot;
    write_graphviz_dp(dot, g, dp, "node_id");
    std::cout << "=== DOT output ===\n" << dot.str();

    // --- Read back ---
    Graph g2;
    dynamic_properties dp2;
    dp2.property("node_id", get(&City::name, g2));
    dp2.property("weight", get(&Road::weight, g2));

    std::istringstream in(dot.str());
    read_graphviz(in, g2, dp2, "node_id");

    std::cout << "=== Read back ===\n";
    std::cout << num_vertices(g2) << " vertices, "
              << num_edges(g2) << " edges\n";
}
