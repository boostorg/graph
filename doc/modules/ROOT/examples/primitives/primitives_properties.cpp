#include <boost/graph/adjacency_list.hpp>
#include <iostream>
#include <string>

using namespace boost;

struct City { std::string name; };
struct Road { double km; };
using Graph = adjacency_list<vecS, vecS, directedS, City, Road>;

int main() {
    Graph g(2);
    add_edge(0, 1, Road{460.0}, g);

    g[0].name = "Paris";                        // direct struct access
    auto km = get(&Road::km, g);                // property map from bundled member
    std::cout << get(km, edge(0, 1, g).first);  // read through property map
}
