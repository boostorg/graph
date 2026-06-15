#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;
    using Edge = graph_traits<Graph>::edge_descriptor;

    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);

    // Record the incoming edge for each vertex
    std::vector<Edge> pred_edge(num_vertices(g));
    auto vis = make_bfs_visitor(
        record_edge_predecessors(pred_edge.data(), on_tree_edge())
    );
    breadth_first_search(g, vertex(0, g), visitor(vis));

    for (std::size_t v = 1; v < num_vertices(g); ++v) {
        Edge e = pred_edge[v];
        std::cout << "vertex " << v << "  arrived via edge "
                  << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
