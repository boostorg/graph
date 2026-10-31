#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/property_map/shared_array_property_map.hpp>
#include <functional>
#include <iostream>
#include <vector>

struct Edge { int weight; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS, boost::no_property, Edge>;
using vertex_descriptor = boost::graph_traits<Graph>::vertex_descriptor;
using weight_type = int;

// records the vertices in the order the search visits them
struct order_recorder : boost::graph::default_mas_visitor {
    std::vector<vertex_descriptor> order;
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

    // keyed max priority queue the search runs on: reach counts plus heap positions
    using index_map_type = boost::property_map<Graph, boost::vertex_index_t>::const_type;
    using distances_map_type = boost::shared_array_property_map<weight_type, index_map_type>;
    using index_in_heap_type = std::vector<vertex_descriptor>::size_type;
    using indices_map_type = boost::shared_array_property_map<index_in_heap_type, index_map_type>;
    using max_priority_queue_type = boost::d_ary_heap_indirect<vertex_descriptor, 4, indices_map_type, distances_map_type, std::greater<weight_type>>;

    auto distances_map = boost::make_shared_array_property_map(boost::num_vertices(g), weight_type(0), boost::get(boost::vertex_index, g));
    auto indices_map = boost::make_shared_array_property_map(boost::num_vertices(g), index_in_heap_type(-1), boost::get(boost::vertex_index, g));
    max_priority_queue_type pq(distances_map, indices_map);

    order_recorder visitor;
    vertex_descriptor start = *boost::vertices(g).first;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::graph::maximum_adjacency_search(g, weight_map, std::ref(visitor), start, pq);

    std::cout << "Visit order:";
    for (vertex_descriptor v : visitor.order) std::cout << ' ' << v;
    std::cout << "\nLast visited vertex (highest connectivity): " << visitor.order.back() << '\n';
}
