#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <iostream>
#include <vector>
#include <algorithm>

struct VertexProps { std::string name; };

using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS, VertexProps>;

int main() {
    Graph g{5};
    g[0].name = "A"; g[1].name = "B"; g[2].name = "C";
    g[3].name = "D"; g[4].name = "E";
    boost::add_edge(0, 2, g);  // A -> C
    boost::add_edge(1, 2, g);  // B -> C
    boost::add_edge(2, 3, g);  // C -> D
    boost::add_edge(2, 4, g);  // C -> E

    std::vector<Graph::vertex_descriptor> order;
    boost::topological_sort(g, std::back_inserter(order));

    std::cout << "Topological order: ";
    for (auto v : order) { std::cout << g[v].name << " "; }
    std::cout << std::endl;
}
