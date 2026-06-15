#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>
#include <iostream>
#include <vector>

using namespace boost;

// Bundled edge data replaces the nested `property<edge_capacity_t, ...>`
// chain. The reverse descriptor field links each edge to its counterpart.
using Traits = adjacency_list_traits<vecS, vecS, directedS>;
struct Edge {
    int capacity;
    int residual_capacity;
    Traits::edge_descriptor reverse;
};

using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;
using Vertex = graph_traits<Graph>::vertex_descriptor;

void add_flow_edge(Graph& g, int from, int to, int cap) {
    Descriptor e = add_edge(from, to, g).first;
    Descriptor r = add_edge(to, from, g).first;
    g[e].capacity = cap; g[e].reverse = r;
    g[r].capacity = 0;   g[r].reverse = e;
}

int main() {
    Graph g(4);
    add_flow_edge(g, 0, 1, 10);
    add_flow_edge(g, 0, 2, 10);
    add_flow_edge(g, 1, 3, 5);
    add_flow_edge(g, 2, 3, 15);
    add_flow_edge(g, 1, 2, 4);

    // 8-arg positional form accepts explicit capacity / residual / reverse maps.
    std::vector<default_color_type> color(num_vertices(g));
    std::vector<Descriptor> pred(num_vertices(g));
    auto color_map = make_iterator_property_map(color.begin(), get(vertex_index, g));
    auto pred_map  = make_iterator_property_map(pred.begin(),  get(vertex_index, g));

    int flow = edmonds_karp_max_flow(g, Vertex(0), Vertex(3),
        get(&Edge::capacity, g),
        get(&Edge::residual_capacity, g),
        get(&Edge::reverse, g),
        color_map, pred_map);
    std::cout << "Edmonds-Karp max flow: " << flow << "\n";
}
