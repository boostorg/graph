#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/maximum_adjacency_search.hpp>
#include <iostream>
#include <vector>

struct Edge { int weight; };

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;

    Graph g(5);
    add_edge(0, 1, Edge{2}, g);
    add_edge(0, 4, Edge{3}, g);
    add_edge(1, 2, Edge{3}, g);
    add_edge(1, 4, Edge{2}, g);
    add_edge(2, 3, Edge{4}, g);
    add_edge(3, 4, Edge{1}, g);

    // Bundled weight via member pointer — passed through the named parameter.
    maximum_adjacency_search(g,
        boost::weight_map(get(&Edge::weight, g)));

    std::cout << "Maximum adjacency search completed\n";
    std::cout << "Last vertex visited has highest connectivity\n";
}
