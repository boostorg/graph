#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/boyer_myrvold_planar_test.hpp>
#include <boost/graph/planar_face_traversal.hpp>
#include <iostream>
#include <vector>

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS,
    property<vertex_index_t, int>, property<edge_index_t, int>>;

struct face_counter : public planar_face_traversal_visitor {
    int count = 0;
    void begin_face() { ++count; }
};

int main() {
    Graph g(4);
    add_edge(0, 1, g); add_edge(1, 2, g);
    add_edge(2, 3, g); add_edge(3, 0, g);
    add_edge(0, 2, g);

    int idx = 0;
    for (auto e : make_iterator_range(edges(g)))
        put(edge_index, g, e, idx++);

    using embedding_t = std::vector<std::vector<graph_traits<Graph>::edge_descriptor>>;
    embedding_t embedding(num_vertices(g));
    boyer_myrvold_planarity_test(boyer_myrvold_params::graph = g,
        boyer_myrvold_params::embedding = &embedding[0]);

    face_counter visitor;
    planar_face_traversal(g, &embedding[0], visitor);
    std::cout << "Number of faces: " << visitor.count << "\n";
}
