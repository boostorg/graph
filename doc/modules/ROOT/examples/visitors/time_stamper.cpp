#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;

    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);

    std::vector<int> dtime(num_vertices(g));
    std::vector<int> ftime(num_vertices(g));
    int t = 0;

    auto vis = make_dfs_visitor(
        std::make_pair(
            stamp_times(dtime.data(), t, on_discover_vertex()),
            stamp_times(ftime.data(), t, on_finish_vertex())
        )
    );
    depth_first_search(g, visitor(vis));

    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v
                  << "  discover=" << dtime[v]
                  << "  finish=" << ftime[v] << "\n";
    }
}
