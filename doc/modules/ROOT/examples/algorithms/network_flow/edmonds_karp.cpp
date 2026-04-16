#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>
#include <iostream>

using namespace boost;

using Traits = adjacency_list_traits<vecS, vecS, directedS>;
using Graph = adjacency_list<vecS, vecS, directedS,
    property<vertex_index_t, std::size_t>,
    property<edge_capacity_t, int,
    property<edge_residual_capacity_t, int,
    property<edge_reverse_t, Traits::edge_descriptor>>>>;
using Edge = Traits::edge_descriptor;

void add_flow_edge(Graph& g, int from, int to, int cap) {
    auto cap_map = get(edge_capacity, g);
    auto rev_map = get(edge_reverse, g);
    Edge e = add_edge(from, to, g).first;
    Edge r = add_edge(to, from, g).first;
    cap_map[e] = cap;
    cap_map[r] = 0;
    rev_map[e] = r;
    rev_map[r] = e;
}

int main() {
    Graph g(4);
    add_flow_edge(g, 0, 1, 10);
    add_flow_edge(g, 0, 2, 10);
    add_flow_edge(g, 1, 3, 5);
    add_flow_edge(g, 2, 3, 15);
    add_flow_edge(g, 1, 2, 4);

    int flow = edmonds_karp_max_flow(g, 0, 3);
    std::cout << "Edmonds-Karp max flow: " << flow << "\n";
}
