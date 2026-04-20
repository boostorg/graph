#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/metric_tsp_approx.hpp>
#include <iostream>
#include <vector>

struct Edge { double weight; };

using namespace boost;
using Graph = adjacency_list<vecS, vecS, undirectedS, no_property, Edge>;
using Vertex = graph_traits<Graph>::vertex_descriptor;

int main() {
    const int N = 4;
    Graph g(N);
    double w[][4] = {{0,10,15,20},{10,0,35,25},{15,35,0,30},{20,25,30,0}};
    for (int i = 0; i < N; ++i)
        for (int j = i + 1; j < N; ++j)
            add_edge(i, j, Edge{w[i][j]}, g);

    // Bundled weight via member pointer — the 3-arg overload of
    // metric_tsp_approx_tour takes an explicit WeightMap.
    std::vector<Vertex> tour;
    metric_tsp_approx_tour(g, get(&Edge::weight, g), std::back_inserter(tour));

    std::cout << "TSP tour:";
    for (auto v : tour) { std::cout << " " << v; }
    std::cout << "\n";
}
