#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/subgraph.hpp>
#include <iostream>

int main() {
    using namespace boost;
    // subgraph requires edge_index_t as an internal property tag
    using Graph = subgraph<adjacency_list<vecS, vecS, directedS,
        no_property, property<edge_index_t, int>>>;

    // Root graph: 5 vertices, 6 edges
    Graph root(5);
    add_edge(0, 1, root);
    add_edge(1, 2, root);
    add_edge(2, 3, root);
    add_edge(3, 4, root);
    add_edge(4, 0, root);
    add_edge(1, 3, root);

    // Create a subgraph containing vertices {1, 2, 3}
    Graph& sub = root.create_subgraph();
    add_vertex(1, sub);  // global vertex 1
    add_vertex(2, sub);  // global vertex 2
    add_vertex(3, sub);  // global vertex 3

    std::cout << "Root: " << num_vertices(root) << " vertices, "
              << num_edges(root) << " edges\n";
    std::cout << "Subgraph: " << num_vertices(sub) << " vertices, "
              << num_edges(sub) << " edges\n\n";

    std::cout << "Subgraph edges (local descriptors):\n";
    for (auto ei = edges(sub).first; ei != edges(sub).second; ++ei) {
        auto s = sub.local_to_global(source(*ei, sub));
        auto t = sub.local_to_global(target(*ei, sub));
        std::cout << "  " << s << " -> " << t << "\n";
    }
}
