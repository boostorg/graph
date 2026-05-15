#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph  = adjacency_list<vecS, vecS, undirectedS>;
    using Vertex = Graph::vertex_descriptor;

    std::vector<std::pair<int,int>> edges = {
        {0,1},{0,3},{0,18},{1,2},{1,19},{2,3},{2,8},{3,4},{4,5},{4,8},
        {5,6},{5,18},{6,7},{6,12},{7,8},{7,11},{8,9},{9,10},{9,19},{10,11},
        {10,15},{11,12},{11,14},{12,13},{13,14},{13,17},{14,15},{14,17},
        {15,16},{16,17},
    };

    Graph g(edges.begin(), edges.end(), 20);
    const Vertex source = 9;
    const Vertex target = 13;

    // Custom external storage
    std::vector<int>    distances(num_vertices(g));
    std::vector<Vertex> predecessors(num_vertices(g));

    // Wrap storage into property maps
    auto vidx = get(vertex_index, g);
    auto dmap = make_iterator_property_map(distances.begin(), vidx);
    auto pmap = make_iterator_property_map(predecessors.begin(), vidx);

    breadth_first_search(g, source,
        visitor(make_bfs_visitor(std::make_pair(
            record_distances(dmap, on_tree_edge()),
            record_predecessors(pmap, on_tree_edge())))));

    // Walk back through the predecessor property map
    for (auto v = target; v != source; v = get(pmap, v))
        std::cout << v << ' ';
    std::cout << source << '\n';
}
