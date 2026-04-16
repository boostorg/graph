#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/geodesic_distance.hpp>
#include <iostream>

struct EdgeProps { int weight; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
    boost::no_property, EdgeProps>;
using DistProp = boost::exterior_vertex_property<Graph, int>;
using DistMatrix = DistProp::matrix_type;
using DistMatrixMap = DistProp::matrix_map_type;
using GeoProp = boost::exterior_vertex_property<Graph, double>;

int main() {
    Graph g{4};
    boost::add_edge(0, 1, EdgeProps{1}, g);
    boost::add_edge(1, 2, EdgeProps{2}, g);
    boost::add_edge(2, 3, EdgeProps{1}, g);
    boost::add_edge(0, 3, EdgeProps{5}, g);

    DistMatrix dist(num_vertices(g));
    DistMatrixMap dm(dist, g);
    boost::floyd_warshall_all_pairs_shortest_paths(g, dm,
        boost::weight_map(get(&EdgeProps::weight, g)));

    GeoProp::container_type geo_vec(num_vertices(g));
    GeoProp::map_type geo(geo_vec, g);
    double graph_mean = boost::all_mean_geodesics(g, dm, geo,
        boost::measure_mean_geodesic(g, GeoProp::map_type(geo_vec, g)));

    for (auto v : boost::make_iterator_range(vertices(g)))
        std::cout << "mean geodesic[" << v << "] = " << geo_vec[v] << "\n";
    std::cout << "Graph mean geodesic: " << graph_mean << "\n";
}
