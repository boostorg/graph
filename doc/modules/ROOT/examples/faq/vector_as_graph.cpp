#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <iostream>
#include <list>
#include <vector>

int main() {
    using namespace boost;
    std::vector<std::list<int>> g(4);
    g[0].push_back(1); g[0].push_back(2);
    g[1].push_back(3);
    g[2].push_back(3);

    std::cout << "V = " << num_vertices(g) << "\n";
    for (std::size_t u = 0; u < g.size(); ++u)
        for (int v : g[u])
            std::cout << "  " << u << " -> " << v << "\n";
}
