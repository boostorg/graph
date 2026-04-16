#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boykov_kolmogorov_max_flow.hpp>
#include <iostream>

using namespace boost;
using Traits = adjacency_list_traits<vecS, vecS, directedS>;
using Graph = adjacency_list<vecS, vecS, directedS,
    property<vertex_color_t, default_color_type,
    property<vertex_distance_t, long,
    property<vertex_predecessor_t, Traits::edge_descriptor>>>,
    property<edge_capacity_t, int,
    property<edge_residual_capacity_t, int,
    property<edge_reverse_t, Traits::edge_descriptor>>>>;
using Edge = graph_traits<Graph>::edge_descriptor;

void add_flow_edge(Graph& g, int u, int v, int cap) {
    Edge e1 = add_edge(u, v, g).first;
    Edge e2 = add_edge(v, u, g).first;
    put(edge_capacity, g, e1, cap);
    put(edge_capacity, g, e2, 0);
    put(edge_reverse, g, e1, e2);
    put(edge_reverse, g, e2, e1);
}

int main() {
    Graph g(4);
    add_flow_edge(g, 0, 1, 3);
    add_flow_edge(g, 0, 2, 2);
    add_flow_edge(g, 1, 3, 2);
    add_flow_edge(g, 2, 3, 3);

    int flow = boykov_kolmogorov_max_flow(g, vertex(0, g), vertex(3, g));
    std::cout << "Boykov-Kolmogorov max flow: " << flow << "\n";
}
