#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS,
        no_property, property<edge_index_t, int>>;
    using Edge = graph_traits<Graph>::edge_descriptor;

    Graph g(4);
    add_edge(0, 1, 0, g);
    add_edge(1, 2, 1, g);
    add_edge(2, 0, 2, g);  // creates a back edge
    add_edge(1, 3, 3, g);

    // Mark back edges with true using property_put
    std::vector<bool> is_back(num_edges(g), false);
    auto edge_id = get(edge_index, g);
    auto back_map = make_iterator_property_map(is_back.begin(), edge_id);

    auto vis = make_dfs_visitor(
        put_property(back_map, true, on_back_edge())
    );
    depth_first_search(g, visitor(vis));

    for (auto ei = edges(g).first; ei != edges(g).second; ++ei) {
        Edge e = *ei;
        std::cout << source(e, g) << " -> " << target(e, g);
        if (is_back[get(edge_id, e)]) {
            std::cout << "  [back edge]";
        }
        std::cout << "\n";
    }
}
