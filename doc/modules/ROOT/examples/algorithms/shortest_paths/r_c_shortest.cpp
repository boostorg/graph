#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/r_c_shortest_paths.hpp>
#include <iostream>
#include <vector>

// r_c_shortest_paths requires an edge_index_map, so we include edge_index_t.
using namespace boost;
using Graph = adjacency_list<vecS, vecS, directedS, no_property,
    property<edge_index_t, int, property<edge_weight_t, int>>>;
using Edge = graph_traits<Graph>::edge_descriptor;

// Resource container: (cost, time)
struct Res { int cost; int time; };
bool operator==(const Res& a, const Res& b) { return a.cost == b.cost && a.time == b.time; }
bool operator<(const Res& a, const Res& b) { return a.cost < b.cost; }

struct ResExt {
    bool operator()(const Graph& g, Res& new_r, const Res& old_r, Edge e) const {
        new_r.cost = old_r.cost + get(edge_weight, g, e);
        new_r.time = old_r.time + 1;
        return new_r.time <= 5; // hop-count constraint
    }
};

struct Dom {
    bool operator()(const Res& r1, const Res& r2) const {
        return r1.cost <= r2.cost && r1.time <= r2.time;
    }
};

int main() {
    Graph g(4);
    int idx = 0;
    auto ae = [&](int u, int v, int w) {
        auto e = add_edge(u, v, g).first;
        put(edge_index, g, e, idx++);
        put(edge_weight, g, e, w);
    };
    ae(0, 1, 2); ae(0, 2, 5); ae(1, 3, 3); ae(2, 3, 2);

    std::vector<Edge> path;
    Res result{0, 0};
    r_c_shortest_paths(g, get(vertex_index, g), get(edge_index, g),
        vertex(0, g), vertex(3, g), path, result,
        Res{0, 0}, ResExt{}, Dom{});

    std::cout << "Cost: " << result.cost << ", hops: " << result.time << "\n";
    std::cout << "Path edges: " << path.size() << "\n";
}
