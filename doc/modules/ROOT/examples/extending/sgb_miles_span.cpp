#include <boost/graph/stanford_graph.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>
#include <iostream>

// Accumulates the Prim distance labels as vertices are finalized.
// Each label is the weight of the edge that joined the vertex to the tree.
template <class DistanceMap>
struct total_length_visitor : boost::dijkstra_visitor<> {
    using length_type = typename boost::property_traits<DistanceMap>::value_type;
    
    total_length_visitor(length_type& t, DistanceMap d) : total(t), distance_map(d) {}
    
    template <class Vertex, class Graph>
    void finish_vertex(Vertex v, const Graph&) { total += boost::get(distance_map, v); }
    
    length_type& total;
    DistanceMap distance_map;
};

int main() {
    // Native Stanford GraphBase graph built from real city mileage data.
    const unsigned long city_count = 100;
    const long north_weight = 0;
    const long west_weight = 0;
    const long population_weight = 0;
    const unsigned long max_distance = 0;
    const unsigned long max_degree = 10;
    const long seed = 0;
    Graph* g = miles(city_count, north_weight, west_weight, population_weight, max_distance, max_degree, seed);
    if (g == nullptr) {
        std::cerr << "Could not build the miles graph, SGB panic code " << panic_code << "\n";
        return 1;
    }

    // Prim reuses the SGB per vertex utility fields as its work maps.
    auto distance_map = boost::get(boost::z_property<long>(), g);
    auto parent_map = boost::get(boost::w_property<Vertex*>(), g);
    auto weight_map = boost::get(boost::edge_length_t(), g);
    auto index_map = boost::get(boost::vertex_index, g);

    long total_length = 0;
    total_length_visitor<decltype(distance_map)> vis(total_length, distance_map);

    auto root = *boost::vertices(g).first;
    boost::prim_minimum_spanning_tree(g, root, parent_map, distance_map, weight_map, index_map, vis);

    std::cout << "Graph ID: " << g->id << "\n";
    std::cout << "Minimum spanning tree length: " << total_length << "\n";

    gb_recycle(g);
}
