#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <iostream>
#include <vector>

struct EdgeProps { int weight; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, boost::no_property, EdgeProps>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{4};
    boost::add_edge(0, 1, EdgeProps{1}, g);
    boost::add_edge(1, 2, EdgeProps{2}, g);
    boost::add_edge(0, 2, EdgeProps{10}, g);
    boost::add_edge(2, 3, EdgeProps{1}, g);

    std::vector<Vertex> pred(num_vertices(g));
    std::vector<int> dist(num_vertices(g));
    auto idx = get(boost::vertex_index, g);
    auto wt = get(&EdgeProps::weight, g);

    boost::dijkstra_shortest_paths_no_color_map(g, vertex(0, g),
        boost::predecessor_map(boost::make_iterator_property_map(pred.begin(), idx))
        .distance_map(boost::make_iterator_property_map(dist.begin(), idx))
        .weight_map(wt));

    for (auto v : boost::make_iterator_range(vertices(g)))
        std::cout << "dist[" << v << "] = " << dist[v] << "\n";
}
