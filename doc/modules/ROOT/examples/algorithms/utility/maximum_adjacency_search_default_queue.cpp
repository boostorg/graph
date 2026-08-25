#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <iostream>
#include <vector>

struct Edge { int weight; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, Edge>;
using vertex_descriptor = boost::graph_traits<Graph>::vertex_descriptor;

// records the vertices in the order the search visits them
struct order_recorder : boost::default_mas_visitor {
    std::vector<vertex_descriptor>& order;
    explicit order_recorder(std::vector<vertex_descriptor>& o) : order(o) {}
    void finish_vertex(vertex_descriptor u, const Graph&) { order.push_back(u); }
};

int main() {
    Graph g(5);
    boost::add_edge(0, 1, Edge{2}, g);
    boost::add_edge(0, 4, Edge{3}, g);
    boost::add_edge(1, 2, Edge{3}, g);
    boost::add_edge(1, 4, Edge{2}, g);
    boost::add_edge(2, 3, Edge{4}, g);
    boost::add_edge(3, 4, Edge{1}, g);

    auto weight_map = boost::get(&Edge::weight, g);

    std::vector<vertex_descriptor> order;
    order_recorder visitor(order);

    boost::graph::maximum_adjacency_search(g, weight_map, visitor, *vertices(g).first);

    std::cout << "Visit order:";
    for (vertex_descriptor v : order) std::cout << ' ' << v;
    std::cout << "\nLast visited vertex (highest connectivity): " << order.back() << '\n';
}
