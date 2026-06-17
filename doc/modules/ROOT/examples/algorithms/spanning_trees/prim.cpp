// Prim Minimum Spanning Tree example
#include <iostream>
#include <vector>
#include <limits>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

struct EdgeWeight { int weight; };

using Graph = boost::adjacency_list<
    boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, EdgeWeight>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g{5};
    auto add = [&](int u, int v, int w) { boost::add_edge(u, v, EdgeWeight{w}, g); };
    add(0, 1, 2); add(0, 3, 1); add(1, 2, 3);
    add(1, 3, 2); add(2, 4, 4); add(3, 4, 6);

    auto n = boost::num_vertices(g);
    std::vector<Vertex> pred(n);
    std::vector<int> dist(n);
    auto index_map = boost::get(boost::vertex_index, g);
    auto pred_map = boost::make_iterator_property_map(pred.begin(), index_map);
    auto dist_map = boost::make_iterator_property_map(dist.begin(), index_map);
    auto weight_map = boost::get(&EdgeWeight::weight, g);

    boost::prim_minimum_spanning_tree(g, *boost::vertices(g).first,
        pred_map, dist_map, weight_map, index_map,
        boost::default_dijkstra_visitor{});

    std::cout << "Predecessor map (vertex : parent):\n";
    for (Vertex v = 0; v < boost::num_vertices(g); ++v) {
        std::cout << "  " << v << " : " << pred[v] << "\n";
    }
}
