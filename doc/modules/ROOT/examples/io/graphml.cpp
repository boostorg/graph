#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphml.hpp>
#include <iostream>
#include <sstream>

struct City { std::string name; };
struct Road { int weight; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;

    // --- Write ---
    Graph g(3);
    g[0].name = "Paris";
    g[1].name = "Lyon";
    g[2].name = "Marseille";
    add_edge(0, 1, Road{10}, g);
    add_edge(1, 2, Road{20}, g);
    add_edge(0, 2, Road{30}, g);

    dynamic_properties dp;
    dp.property("name", get(&City::name, g));
    dp.property("weight", get(&Road::weight, g));

    std::ostringstream xml;
    write_graphml(xml, g, dp, true);
    std::cout << "=== GraphML output ===\n" << xml.str() << "\n";

    // --- Read back ---
    Graph g2;
    dynamic_properties dp2;
    dp2.property("name", get(&City::name, g2));
    dp2.property("weight", get(&Road::weight, g2));

    std::istringstream in(xml.str());
    read_graphml(in, g2, dp2);

    std::cout << "=== Read back ===\n";
    std::cout << num_vertices(g2) << " vertices, "
              << num_edges(g2) << " edges\n";
    for (auto v : make_iterator_range(vertices(g2))) {
        std::cout << "  " << g2[v].name << "\n";
    }
}
