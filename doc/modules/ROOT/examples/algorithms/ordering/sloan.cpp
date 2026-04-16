#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/sloan_ordering.hpp>
#include <boost/graph/properties.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<setS, vecS, undirectedS,
    property<vertex_color_t, default_color_type,
    property<vertex_degree_t, int,
    property<vertex_priority_t, double>>>>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(6);
    add_edge(0, 1, g); add_edge(0, 2, g); add_edge(1, 3, g);
    add_edge(2, 3, g); add_edge(3, 4, g); add_edge(4, 5, g);

    std::vector<Vertex> order(num_vertices(g));
    sloan_ordering(g, order.begin(),
        get(vertex_color, g), get(vertex_degree, g), get(vertex_priority, g));

    std::cout << "Sloan ordering:";
    for (auto v : order) { std::cout << " " << v; }
    std::cout << "\n";
}
