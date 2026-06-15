#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/property_map/function_property_map.hpp>
#include <boost/property_map/transform_value_property_map.hpp>
#include <iostream>
#include <map>
#include <string>
#include <vector>

struct City { std::string name; };
struct Road { double km; };

using namespace boost;
using Graph  = adjacency_list<vecS, vecS, directedS, City, Road>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    Graph g(3);
    g[0].name = "Paris";
    g[1].name = "Lyon";
    g[2].name = "Marseille";
    add_edge(0, 1, Road{460}, g);
    add_edge(1, 2, Road{310}, g);
    add_edge(0, 2, Road{775}, g);

    auto index_map = get(vertex_index, g);

    // Bundled property map
    auto km_map = get(&Road::km, g);

    // std::vector + make_iterator_property_map
    std::vector<int> vec(num_vertices(g), -1);
    auto vec_map = make_iterator_property_map(vec.begin(), index_map);
    put(vec_map, vertex(0, g), 42);

    // Raw C array (usable directly as a property map)
    int arr[3] = {100, 200, 300};

    // std::map + make_assoc_property_map
    std::map<Vertex, std::string> labels;
    auto label_map = make_assoc_property_map(labels);
    put(label_map, vertex(0, g), "capital");

    // vector_property_map (auto-growing, heap-allocated)
    vector_property_map<double, decltype(index_map)> vpm(num_vertices(g), index_map);
    put(vpm, vertex(2, g), 3.14);

    // Computed from a lambda (read-only)
    auto degree_map = make_function_property_map<Vertex>([&](Vertex v) { return out_degree(v, g); });

    // Transformed view of another map
    auto miles_map = make_transform_value_property_map([](double d) { return d * 0.621371; }, km_map);

    // Constant for all keys
    static_property_map<double> unit(1.0);

    // Discard writes (placeholder)
    dummy_property_map sink;
    put(sink, vertex(0, g), 999); // silently ignored

    // All accessed the same way: get(map, key)
    std::cout << get(km_map, edge(0, 1, g).first) << " km\n";
    std::cout << get(vec_map, vertex(0, g)) << "\n";
    std::cout << get(arr, 1) << "\n";
    std::cout << get(label_map, vertex(0, g)) << "\n";
    std::cout << get(vpm, vertex(2, g)) << "\n";
    std::cout << get(degree_map, vertex(0, g)) << "\n";
    std::cout << get(miles_map, edge(0, 1, g).first) << " miles\n";
    std::cout << get(unit, vertex(0, g)) << "\n";
}
