// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example: Finding all maximal cliques with Bron-Kerbosch algorithm

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bron_kerbosch_all_cliques.hpp>
#include <algorithm>
#include <iostream>
#include <vector>

// Undirected graph with no properties
using Graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

// Custom visitor that prints each maximal clique found.
// The clique is passed as a deque of vertex descriptors.
struct PrintCliquesVisitor
{
    template <typename Clique, typename G>
    void clique(const Clique& c, const G& /*g*/)
    {
        // Copy to a vector and sort so output is deterministic
        std::vector<Vertex> sorted(c.begin(), c.end());
        std::sort(sorted.begin(), sorted.end());

        std::cout << "Clique: {";
        for (std::size_t i = 0; i < sorted.size(); ++i)
        {
            if (i > 0)
            {
                std::cout << ", ";
            }
            std::cout << sorted[i];
        }
        std::cout << "}" << std::endl;
    }
};

int main()
{
    // Build an undirected graph with 4 vertices:
    //
    //   0 --- 1
    //   |   / |
    //   |  /  |
    //   | /   |
    //   2 --- 3
    //
    // Edges: 0-1, 0-2, 1-2, 1-3, 2-3
    // Triangle {0,1,2} and triangle {1,2,3} are the maximal cliques.

    Graph g{4};

    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(1, 3, g);
    boost::add_edge(2, 3, g);

    std::cout << "Finding all maximal cliques:" << std::endl;
    boost::bron_kerbosch_all_cliques(g, PrintCliquesVisitor{});

    return 0;
}
