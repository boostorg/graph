// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example: Finding all elementary cycles with Tiernan's algorithm

#include <boost/graph/directed_graph.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <iostream>
#include <vector>

// Directed graph with no bundled properties
using Graph = boost::directed_graph<>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

// Custom visitor that prints each cycle found.
// The cycle is passed as a const vector of vertex descriptors.
struct PrintCyclesVisitor
{
    template <typename Path, typename G>
    void cycle(const Path& p, const G& g)
    {
        std::cout << "Cycle: ";
        for (std::size_t i = 0; i < p.size(); ++i)
        {
            if (i > 0)
            {
                std::cout << " -> ";
            }
            std::cout << boost::get(boost::vertex_index, g, p[i]);
        }
        std::cout << " -> " << boost::get(boost::vertex_index, g, p.front())
                  << std::endl;
    }
};

int main()
{
    // Build a directed graph with 4 vertices and two cycles:
    //
    //   0 --> 1 --> 2
    //   ^     ^     |
    //   |     |     |
    //   +-----+-----+
    //         |
    //         3
    //
    // Edges: 0->1, 1->2, 2->0, 1->3, 3->1
    // Cycle 1: 0 -> 1 -> 2 -> 0
    // Cycle 2: 1 -> 3 -> 1

    Graph g;

    Vertex v0 = g.add_vertex();
    Vertex v1 = g.add_vertex();
    Vertex v2 = g.add_vertex();
    Vertex v3 = g.add_vertex();

    g.add_edge(v0, v1);
    g.add_edge(v1, v2);
    g.add_edge(v2, v0);
    g.add_edge(v1, v3);
    g.add_edge(v3, v1);

    std::cout << "Finding all elementary cycles:" << std::endl;
    boost::tiernan_all_cycles(g, PrintCyclesVisitor{});

    return 0;
}
