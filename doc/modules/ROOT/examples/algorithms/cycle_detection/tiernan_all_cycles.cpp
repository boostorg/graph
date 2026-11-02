// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Example: Finding all elementary cycles with Tiernan's algorithm

#include <boost/graph/directed_graph.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Directed graph with no bundled properties
using Graph = boost::directed_graph<>;
using Vertex = boost::graph_traits<Graph>::vertex_descriptor;

// Custom visitor that records each cycle found.
// The cycle is passed as a const vector of vertex descriptors.
struct RecordCyclesVisitor
{
    std::vector<std::string> cycles;

    template <typename Path, typename G>
    void cycle(const Path& p, const G& g)
    {
        std::ostringstream out;
        out << "Cycle: ";
        for (std::size_t i = 0; i < p.size(); ++i)
        {
            if (i > 0)
            {
                out << " -> ";
            }
            out << boost::get(boost::vertex_index, g, p[i]);
        }
        out << " -> " << boost::get(boost::vertex_index, g, p.front());
        cycles.push_back(out.str());
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

    RecordCyclesVisitor visitor;

    // std::ref lets the visitor keep its state across the copy the algorithm makes
    boost::tiernan_all_cycles(g, std::ref(visitor));

    std::cout << "Finding all elementary cycles:" << std::endl;
    for (const std::string& cycle : visitor.cycles)
    {
        std::cout << cycle << std::endl;
    }

    return 0;
}
