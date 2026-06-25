#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/biconnected_components.hpp>
#include <iostream>

struct Edge { int comp; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 0, g);  // triangle
    add_edge(2, 3, g);
    add_edge(3, 4, g);
    add_edge(4, 2, g);  // second triangle

    // Bundled component index written via member-pointer property map.
    auto num = biconnected_components(g, get(&Edge::comp, g));

    std::cout << "Biconnected components: " << num << "\n";
    for (auto ei = edges(g).first; ei != edges(g).second; ++ei) {
        std::cout << "  " << source(*ei, g) << "-" << target(*ei, g)
                  << " component " << g[*ei].comp << "\n";
    }
}
