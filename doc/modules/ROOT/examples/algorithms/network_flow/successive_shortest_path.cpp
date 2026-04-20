#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/successive_shortest_path_nonnegative_weights.hpp>
#include <boost/graph/find_flow_cost.hpp>
#include <iostream>
#include <vector>

using namespace boost;

using Traits = adjacency_list_traits<vecS, vecS, directedS>;
struct Edge {
    int capacity;
    int residual_capacity;
    int weight;
    Traits::edge_descriptor reverse;
};

using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;
using Vertex = graph_traits<Graph>::vertex_descriptor;

void add_edge_pair(Graph& g, int u, int v, int cap, int cost) {
    Descriptor e = add_edge(u, v, g).first;
    Descriptor r = add_edge(v, u, g).first;
    g[e].capacity = cap;  g[r].capacity = 0;
    g[e].weight = cost;   g[r].weight = -cost;
    g[e].reverse = r;     g[r].reverse = e;
    g[e].residual_capacity = 0;
    g[r].residual_capacity = 0;
}

int main() {
    Graph g(4);
    add_edge_pair(g, 0, 1, 2, 1);
    add_edge_pair(g, 0, 2, 1, 3);
    add_edge_pair(g, 1, 3, 2, 2);
    add_edge_pair(g, 2, 3, 3, 1);

    auto cap = get(&Edge::capacity, g);
    auto res = get(&Edge::residual_capacity, g);
    auto wgt = get(&Edge::weight, g);
    auto rev = get(&Edge::reverse, g);
    auto idx = get(vertex_index, g);

    std::vector<Descriptor> pred(num_vertices(g));
    std::vector<int> dist(num_vertices(g));
    std::vector<int> dist2(num_vertices(g));

    successive_shortest_path_nonnegative_weights(g, Vertex(0), Vertex(3),
        cap, res, wgt, rev, idx,
        make_iterator_property_map(pred.begin(), idx),
        make_iterator_property_map(dist.begin(), idx),
        make_iterator_property_map(dist2.begin(), idx));

    std::cout << "Min cost: " << find_flow_cost(g, cap, res, wgt) << "\n";
}
