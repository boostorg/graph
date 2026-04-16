#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exterior_property.hpp>
#include <boost/graph/floyd_warshall_shortest.hpp>
#include <boost/graph/eccentricity.hpp>
#include <iostream>

struct EdgeProps { int weight; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
    boost::no_property, EdgeProps>;
using DistProp = boost::exterior_vertex_property<Graph, int>;
using DistMatrix = DistProp::matrix_type;
using DistMatrixMap = DistProp::matrix_map_type;
using EccMap = boost::exterior_vertex_property<Graph, int>;

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

    EccMap::container_type ecc_vec(num_vertices(g));
    EccMap::map_type ecc(ecc_vec, g);
    auto rd = boost::all_eccentricities(g, dm, ecc);

    std::cout << "Radius: " << rd.first << ", Diameter: " << rd.second << "\n";
    for (auto v : boost::make_iterator_range(vertices(g)))
        std::cout << "eccentricity[" << v << "] = " << ecc_vec[v] << "\n";
}
