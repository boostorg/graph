#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>
#include <utility>
#include <vector>

struct City { std::string name; };
struct Road { double km; };

int main() {
    using namespace boost;
    using Graph = compressed_sparse_row_graph<directedS, City, Road>;

    std::vector<std::pair<int,int>> edges = {
        {0,1}, {0,2}, {1,2}, {2,3}
    };
    std::vector<Road> props = {
        {460}, {775}, {310}, {200}
    };

    Graph g(edges_are_unsorted, edges.begin(), edges.end(),
            props.begin(), 4);

    g[0].name = "Paris";
    g[1].name = "Lyon";
    g[2].name = "Marseille";
    g[3].name = "Nice";

    for (auto v : make_iterator_range(vertices(g))) {
        for (auto e : make_iterator_range(out_edges(v, g))) {
            std::cout << g[source(e, g)].name << " -> "
                      << g[target(e, g)].name
                      << " (" << g[e].km << " km)\n";
        }
    }
}
