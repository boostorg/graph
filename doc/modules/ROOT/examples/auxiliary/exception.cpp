#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/exception.hpp>
#include <iostream>
#include <vector>

int main() {
    using namespace boost;
    using Graph = adjacency_list<vecS, vecS, directedS>;

    Graph g(3);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 0, g);  // creates a cycle

    std::vector<int> order;
    try {
        topological_sort(g, std::back_inserter(order));
    } catch (const not_a_dag& e) {
        std::cout << "Exception: " << e.what() << "\n";
    }
}
