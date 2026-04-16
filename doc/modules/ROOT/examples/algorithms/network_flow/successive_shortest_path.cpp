#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/successive_shortest_path_nonnegative_weights.hpp>
#include <boost/graph/find_flow_cost.hpp>
#include <iostream>

using namespace boost;

// This algorithm requires internal property tags for edge properties.
using Traits = adjacency_list_traits<vecS, vecS, directedS>;
using Graph = adjacency_list<vecS, vecS, directedS, no_property,
    property<edge_capacity_t, int,
    property<edge_residual_capacity_t, int,
    property<edge_weight_t, int,
    property<edge_reverse_t, Traits::edge_descriptor>>>>>;
using Edge = Traits::edge_descriptor;

void add_edge_pair(Graph& g, int u, int v, int cap, int cost) {
    Edge e = add_edge(u, v, g).first;
    Edge r = add_edge(v, u, g).first;
    put(edge_capacity, g, e, cap);  put(edge_capacity, g, r, 0);
    put(edge_weight, g, e, cost);   put(edge_weight, g, r, -cost);
    put(edge_reverse, g, e, r);     put(edge_reverse, g, r, e);
    put(edge_residual_capacity, g, e, 0);
    put(edge_residual_capacity, g, r, 0);
}

int main() {
    Graph g(4);
    add_edge_pair(g, 0, 1, 2, 1);
    add_edge_pair(g, 0, 2, 1, 3);
    add_edge_pair(g, 1, 3, 2, 2);
    add_edge_pair(g, 2, 3, 3, 1);

    successive_shortest_path_nonnegative_weights(g, 0, 3);
    std::cout << "Min cost: " << find_flow_cost(g) << "\n";
}
