#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>
#include <iostream>
#include <vector>
#include <limits>
#include <cmath>

struct Road { double km; };

// Heuristic: straight-line distance (here just returns 0 for simplicity)
struct zero_heuristic : public boost::astar_heuristic<
    boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
        boost::no_property, Road>, double>
{
    double operator()(unsigned long) const { return 0.0; }
};

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS, no_property, Road>;
    using Vertex = graph_traits<Graph>::vertex_descriptor;

    Graph g(4);
    add_edge(0, 1, Road{1.0}, g);
    add_edge(1, 2, Road{2.0}, g);
    add_edge(0, 2, Road{5.0}, g);
    add_edge(2, 3, Road{1.0}, g);

    std::vector<Vertex> pred(num_vertices(g));
    std::vector<double> dist(num_vertices(g),
                             (std::numeric_limits<double>::max)());
    auto index = get(vertex_index, g);
    auto pred_map = make_iterator_property_map(pred.begin(), index);
    auto dist_map = make_iterator_property_map(dist.begin(), index);

    auto vis = make_astar_visitor(
        record_predecessors(pred_map, on_edge_relaxed())
    );

    astar_search(g, vertex(0, g), zero_heuristic(),
        visitor(vis).
        weight_map(get(&Road::km, g)).
        distance_map(dist_map).
        predecessor_map(pred_map));

    for (std::size_t v = 0; v < num_vertices(g); ++v) {
        std::cout << "vertex " << v
                  << "  dist=" << dist[v]
                  << "  pred=" << pred[v] << "\n";
    }
}
