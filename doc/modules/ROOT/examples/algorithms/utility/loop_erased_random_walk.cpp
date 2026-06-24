#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/loop_erased_random_walk.hpp>
#include <boost/graph/random.hpp>
#include <iostream>
#include <vector>
#include <random>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(5);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    add_edge(3, 4, g);
    add_edge(1, 3, g);  // shortcut

    std::mt19937 gen(42);
    auto next_edge = [&](Vertex v, const Graph& gr) {
        auto range = out_edges(v, gr);
        auto n = std::distance(range.first, range.second);
        std::advance(range.first, gen() % n);
        return *range.first;
    };

    std::vector<default_color_type> colors(num_vertices(g), white_color);
    colors[4] = black_color;  // target

    std::vector<Vertex> path;
    loop_erased_random_walk(g, vertex(0, g), next_edge,
        make_iterator_property_map(colors.begin(), get(vertex_index, g)),
        path);

    std::cout << "Path from 0 to 4: ";
    for (auto v : path) { std::cout << v << " "; }
    std::cout << "\n";
}
