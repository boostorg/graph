#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS>;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(3, 4, g);

    std::vector<int> dist(num_vertices(g), 0);
    auto vis = make_bfs_visitor(
        record_distances(dist.data(), on_tree_edge())
    );
    breadth_first_search(g, vertex(0, g), visitor(vis));

    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v << "  distance=" << dist[v] << "\n";
    }
}
