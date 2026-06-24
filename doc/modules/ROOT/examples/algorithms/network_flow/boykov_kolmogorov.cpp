#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <iostream>

using namespace boost;

using Traits = adjacency_list_traits<vecS, vecS, directedS>;
struct Edge {
    int capacity;
    int residual_capacity;
    Traits::edge_descriptor reverse;
};

using Graph = adjacency_list<vecS, vecS, directedS, no_property, Edge>;
using Descriptor = graph_traits<Graph>::edge_descriptor;

void add_flow_edge(Graph& g, int u, int v, int cap) {
    Descriptor e1 = add_edge(u, v, g).first;
    Descriptor e2 = add_edge(v, u, g).first;
    g[e1].capacity = cap; g[e1].reverse = e2;
    g[e2].capacity = 0;   g[e2].reverse = e1;
}

int main() {
    Graph g(4);
    add_flow_edge(g, 0, 1, 3);
    add_flow_edge(g, 0, 2, 2);
    add_flow_edge(g, 1, 3, 2);
    add_flow_edge(g, 2, 3, 3);

    // 7-arg positional form accepts explicit capacity/residual/reverse maps
    // and allocates predecessor/color/distance internally.
    int flow = boykov_kolmogorov_max_flow(g,
        get(&Edge::capacity, g),
        get(&Edge::residual_capacity, g),
        get(&Edge::reverse, g),
        get(vertex_index, g),
        vertex(0, g), vertex(3, g));
    std::cout << "Boykov-Kolmogorov max flow: " << flow << "\n";
}
