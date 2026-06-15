#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/mesh_graph_generator.hpp>
#include <boost/range/iterator_range.hpp>
#include <iostream>

int main() {
    using namespace boost;
    using Graph = adjacency_list<>;
    using MeshGen = mesh_iterator<Graph>;

    bool toroidal = false;
    Graph g(MeshGen(2, 3, toroidal), MeshGen(), 6);

    std::cout << num_vertices(g) << " vertices, " << num_edges(g) << " edges\n";
    for (auto e : make_iterator_range(edges(g))) {
        std::cout << "  " << source(e, g) << " -> " << target(e, g) << "\n";
    }
}
