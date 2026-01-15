#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/louvain_clustering.hpp>
#include <boost/graph/louvain_quality_functions.hpp>
#include <fstream>
#include <iostream>
#include <vector>
#include <random>
#include <string>

typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
                               boost::no_property,
                               boost::property<boost::edge_weight_t, double>> Graph;

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <edgelist_file> <seed>" << std::endl;
        return 1;
    }
    
    std::string edgelist_file = argv[1];
    int seed = std::atoi(argv[2]);
    
    std::ifstream in(edgelist_file);
    int n_vertices, n_edges;
    in >> n_vertices >> n_edges;
    
    Graph g(n_vertices);
    auto weight_map = get(boost::edge_weight, g);
    
    for (int i = 0; i < n_edges; ++i) {
        int u, v;
        double w;
        in >> u >> v >> w;
        auto e = add_edge(u, v, g).first;
        weight_map[e] = w;
    }
    in.close();
    
    // Create component map
    std::vector<int> components(n_vertices);
    auto component_map = boost::make_iterator_property_map(
        components.begin(), get(boost::vertex_index, g));
    
    // Use 0.0 for both inner and outer loops (matches igraph - most aggressive)
    double Q = boost::louvain_clustering(g, component_map, weight_map, 0.0, 0.0, seed);
    
    std::cout << Q << std::endl;
    for (size_t i = 0; i < components.size(); ++i) {
        std::cout << components[i] << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
