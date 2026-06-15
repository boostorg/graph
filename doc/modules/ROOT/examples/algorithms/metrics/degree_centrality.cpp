#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/degree_centrality.hpp>
#include <iostream>

// Bundled vertex property
struct VertexProps {
    std::string name;
};

int main() {
    // Bidirectional graph with bundled vertex properties
    // (bidirectionalS is needed so that in-degree / prestige can be computed)
    using Graph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::bidirectionalS, VertexProps>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

    Graph g(5);
    g[0].name = "Alice";
    g[1].name = "Bob";
    g[2].name = "Carol";
    g[3].name = "Dave";
    g[4].name = "Eve";

    // Build a small directed network:
    //   Alice -> Bob, Alice -> Carol, Alice -> Dave
    //   Bob -> Carol
    //   Carol -> Dave, Carol -> Eve
    //   Dave -> Eve
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 3, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    add_edge(2, 4, g);
    add_edge(3, 4, g);

    std::cout << "Degree centrality (influence = out-degree):\n";
    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        auto c = boost::degree_centrality(g, v, boost::measure_influence(g));
        std::cout << "  " << g[v].name << ": " << c << "\n";
    }

    std::cout << "\nDegree centrality (prestige = in-degree):\n";
    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        auto c = boost::degree_centrality(g, v, boost::measure_prestige(g));
        std::cout << "  " << g[v].name << ": " << c << "\n";
    }
}
