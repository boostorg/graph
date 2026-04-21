#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<listS, listS, undirectedS,
                                 property<vertex_index_t, std::size_t>>;
    Graph g(4);

    std::size_t i = 0;
    for (auto v : make_iterator_range(vertices(g)))
        put(get(vertex_index, g), v, i++);

    auto it = vertices(g).first;
    auto s = *it; auto t = *++it;
    add_edge(s, t, g);

    breadth_first_search(g, s, visitor(default_bfs_visitor()));
    std::cout << "bfs ok; vertex_index assigned to " << i << " vertices\n";
}
