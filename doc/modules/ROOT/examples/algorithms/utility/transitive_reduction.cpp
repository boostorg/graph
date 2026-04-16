#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/transitive_reduction.hpp>

int main() {
    using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS>;
    using Vertex = boost::graph_traits<Graph>::vertex_descriptor;
    Graph g(4);
    boost::add_edge(0, 1, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(0, 2, g); // redundant: 0->1->2
    boost::add_edge(2, 3, g);
    boost::add_edge(0, 3, g); // redundant: 0->1->2->3

    std::cout << "Before (" << num_edges(g) << " edges):";
    for (auto ep = edges(g); ep.first != ep.second; ++ep.first)
        std::cout << " " << source(*ep.first, g) << "->" << target(*ep.first, g);
    std::cout << "\n";

    Graph tr;
    std::vector<Vertex> g_to_tr(num_vertices(g));
    boost::transitive_reduction(g, tr,
        boost::make_iterator_property_map(g_to_tr.begin(), get(boost::vertex_index, g)),
        get(boost::vertex_index, g));

    std::cout << "After  (" << num_edges(tr) << " edges):";
    for (auto ep = edges(tr); ep.first != ep.second; ++ep.first)
        std::cout << " " << source(*ep.first, tr) << "->" << target(*ep.first, tr);
    std::cout << "\n";
}
