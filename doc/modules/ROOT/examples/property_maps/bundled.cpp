#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

struct City {
    std::string name;
};

struct Road {
    double km;
};

using namespace boost;

using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    // Build a small graph with bundled properties
    Graph g(4);
    g[0].name = "Paris";
    g[1].name = "Lyon";
    g[2].name = "Marseille";
    g[3].name = "Nice";

    add_edge(0, 1, Road{460}, g);
    add_edge(1, 2, Road{310}, g);
    add_edge(0, 2, Road{775}, g);
    add_edge(2, 3, Road{200}, g);

    // Direct access via operator[]
    std::cout << "Source city: " << g[0].name << "\n";

    // Property map access via get(&Struct::member, g)
    auto weight_map = get(&Road::km, g);

    // Prepare predecessor and distance maps
    std::vector<Vertex> pred(num_vertices(g));
    std::vector<double> dist(num_vertices(g));

    auto pred_map = make_iterator_property_map(pred.begin(), get(vertex_index, g));
    auto dist_map = make_iterator_property_map(dist.begin(), get(vertex_index, g));

    // Run Dijkstra using the positional overload with bundled weight map
    Vertex source = 0;
    dijkstra_shortest_paths(g, source,
        pred_map,
        dist_map,
        weight_map,
        get(vertex_index, g),
        std::less<double>(),
        std::plus<double>(),
        (std::numeric_limits<double>::max)(),
        double(0),
        make_dijkstra_visitor(null_visitor()));

    // Print shortest distances from source
    std::cout << "Shortest distances from " << g[source].name << ":\n";
    for (auto v : make_iterator_range(vertices(g))) {
        std::cout << "  " << g[v].name << ": " << dist[v] << " km\n";
    }
}
