#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
    using Vertex = Graph::vertex_descriptor;

    // Undirected graph given as a list of (source, target) vertex-index pairs
    std::vector<std::pair<int,int>> edges = {
        {0,1},{0,3},{0,18},{1,2},{1,19},{2,3},{2,8},{3,4},{4,5},{4,8},
        {5,6},{5,18},{6,7},{6,12},{7,8},{7,11},{8,9},{9,10},{9,19},{10,11},
        {10,15},{11,12},{11,14},{12,13},{13,14},{13,17},{14,15},{14,17},
        {15,16},{16,17},
    };

    // Build the graph from the edge list
    constexpr int num_vertices = 20;
    Graph g(edges.begin(), edges.end(), num_vertices);
    const Vertex source = 9;
    const Vertex target = 13;

    // External storage for the results, one slot per vertex
    std::vector<int> distance_storage(boost::num_vertices(g));
    std::vector<Vertex> predecessor_storage(boost::num_vertices(g));

    // Three property maps:
    // vertex -> its index (provided by the graph type)
    auto vertex_index_map = boost::get(boost::vertex_index, g);
    // vertex -> BFS distance from the source (wraps distance_storage)
    auto distance = boost::make_iterator_property_map(distance_storage.begin(), vertex_index_map);
    // vertex -> its parent in the BFS tree (wraps predecessor_storage)
    auto predecessor = boost::make_iterator_property_map(predecessor_storage.begin(), vertex_index_map);

    boost::breadth_first_search(g, source,
        boost::visitor(boost::make_bfs_visitor(std::make_pair(
            boost::record_distances(distance, boost::on_tree_edge()),
            boost::record_predecessors(predecessor, boost::on_tree_edge())))));

    // Walk back from target to source through the predecessor map
    for (auto v = target; v != source; v = boost::get(predecessor, v))
        std::cout << v << ' ';
    std::cout << source << '\n';
}
