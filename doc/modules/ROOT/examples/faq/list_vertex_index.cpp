#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

using namespace boost;
struct V { std::size_t idx; };
using Graph = adjacency_list<listS, listS, undirectedS, V>;

int main() {
    Graph g;
    for (std::size_t i = 0; i < 4; ++i) add_vertex(V{i}, g);

    auto it = vertices(g).first;
    auto s = *it; auto t = *++it;
    add_edge(s, t, g);

    breadth_first_search(g, s,
        visitor(default_bfs_visitor())
            .vertex_index_map(get(&V::idx, g)));

    std::cout << "bfs ok on listS using bundled vertex_index_map\n";
}
