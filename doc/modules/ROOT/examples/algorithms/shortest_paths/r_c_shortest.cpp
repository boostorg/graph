#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/r_c_shortest_paths.hpp>
#include <iostream>
#include <vector>

using namespace boost;

// Bundled edge fields replace `property<edge_index_t, int, property<edge_weight_t, int>>`.
struct Edge { int idx; int weight; };

using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;

// Resource container: (cost, time)
struct Res { int cost; int time; };
bool operator==(const Res& a, const Res& b) { return a.cost == b.cost && a.time == b.time; }
bool operator<(const Res& a, const Res& b) { return a.cost < b.cost; }

struct ResExt {
    bool operator()(const Graph& g, Res& new_r, const Res& old_r, Descriptor e) const {
        new_r.cost = old_r.cost + g[e].weight;
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
    int i = 0;
    auto ae = [&](int u, int v, int w) {
        auto e = add_edge(u, v, g).first;
        g[e].idx = i++;
        g[e].weight = w;
    };
    ae(0, 1, 2); ae(0, 2, 5); ae(1, 3, 3); ae(2, 3, 2);

    std::vector<Descriptor> path;
    Res result{0, 0};
    r_c_shortest_paths(g, get(vertex_index, g), 
        vertex(0, g), vertex(3, g), path, result,
        Res{0, 0}, ResExt{}, Dom{});

    std::cout << "Cost: " << result.cost << ", hops: " << result.time << "\n";
    std::cout << "Path edges: " << path.size() << "\n";
}
