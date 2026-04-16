#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/astar_search.hpp>
#include <iostream>
#include <vector>
#include <limits>

struct Edge { int weight; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

struct zero_heuristic : public astar_heuristic<Graph, int> {
    int operator()(Vertex) const { return 0; }
};

// Stop when target is found
struct found_target { Vertex v; };
struct target_visitor : public default_astar_visitor {
    target_visitor(Vertex t) : m_target{t} {}
    void examine_vertex(Vertex u, const Graph&) const {
        if (u == m_target) { throw found_target{u}; }
    }
    Vertex m_target;
};

int main() {
    Graph g{4};
    add_edge(0, 1, Edge{2}, g);
    add_edge(1, 2, Edge{3}, g);
    add_edge(0, 2, Edge{8}, g);
    add_edge(2, 3, Edge{1}, g);

    using CostType = int;
    using IndexMap = decltype(get(vertex_index, std::declval<Graph&>()));

    auto n = num_vertices(g);
    std::vector<CostType> dist(n, (std::numeric_limits<CostType>::max)());
    std::vector<CostType> cost(n, (std::numeric_limits<CostType>::max)());
    std::vector<Vertex> pred(n);

    auto index_map = get(vertex_index, g);
    auto dist_map = make_iterator_property_map(dist.begin(), index_map);
    auto cost_map = make_iterator_property_map(cost.begin(), index_map);
    auto pred_map = make_iterator_property_map(pred.begin(), index_map);
    auto weight_map = get(&Edge::weight, g);
    two_bit_color_map<IndexMap> color_map{n, index_map};

    CostType inf = (std::numeric_limits<CostType>::max)();
    CostType zero_val = CostType{};

    try {
        astar_search(g, vertex(0, g), zero_heuristic(),
            target_visitor{3},
            pred_map, cost_map, dist_map, weight_map,
            index_map, color_map,
            std::less<CostType>{}, closed_plus<CostType>{inf},
            inf, zero_val);
    } catch (const found_target&) {}

    for (auto v : make_iterator_range(vertices(g))) {
        std::cout << "distance to " << v << " = " << dist[v] << "\n";
    }
}
