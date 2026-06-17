#include <boost/graph/adjacency_list.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

struct City { std::string name; };
struct Road { double km; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;

void print_graph(const Graph& g) {
    for (auto v : make_iterator_range(vertices(g))) {
        for (auto e : make_iterator_range(out_edges(v, g))) {
            std::cout << "  " << g[source(e, g)].name
                      << " -> " << g[target(e, g)].name
                      << " (" << g[e].km << " km)\n";
        }
    }
}

int main() {
    Graph g;
    auto paris     = add_vertex(City{"Paris"}, g);
    auto lyon      = add_vertex(City{"Lyon"}, g);
    auto marseille = add_vertex(City{"Marseille"}, g);
    add_edge(paris, lyon, Road{460}, g);
    add_edge(paris, marseille, Road{775}, g);
    add_edge(lyon, marseille, Road{310}, g);

    std::cout << "3 vertices, " << num_edges(g) << " edges:\n";
    print_graph(g);

    remove_edge(paris, marseille, g);
    std::cout << "\nAfter remove_edge(paris, marseille):\n";
    print_graph(g);

    auto ei = out_edges(lyon, g).first;
    remove_edge(ei, g);
    std::cout << "\nAfter remove_edge(iterator):\n";
    print_graph(g);
}
