#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/core_numbers.hpp>
#include <iostream>
#include <vector>

// Bundled vertex property
struct VertexProps {
    std::string label;
};

int main() {
    // Undirected graph with bundled vertex properties
    using Graph = boost::adjacency_list<
        boost::vecS, boost::vecS, boost::undirectedS, VertexProps>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    using SizeType = boost::graph_traits<Graph>::vertices_size_type;

    Graph g(7);
    g[0].label = "a";
    g[1].label = "b";
    g[2].label = "c";
    g[3].label = "d";
    g[4].label = "e";
    g[5].label = "f";
    g[6].label = "g";

    // Dense 4-clique: a-b-c-d (vertices 0,1,2,3)
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(0, 3, g);
    add_edge(1, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 3, g);

    // Peripheral vertices attached loosely
    add_edge(3, 4, g);  // d -- e
    add_edge(4, 5, g);  // e -- f
    add_edge(5, 6, g);  // f -- g

    // Compute core numbers into a vector property map
    std::vector<SizeType> core_nums(boost::num_vertices(g));
    auto core_map = boost::make_iterator_property_map(
        core_nums.begin(), boost::get(boost::vertex_index, g));

    boost::core_numbers(g, core_map);

    std::cout << "Core numbers (k-core decomposition):\n";
    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        std::cout << "  vertex " << v
                  << " (" << g[v].label << "): "
                  << core_nums[v] << "\n";
    }
}
