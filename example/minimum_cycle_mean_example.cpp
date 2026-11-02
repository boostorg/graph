#include <iostream>
#include <boost/graph/karp_minimum_cycle_mean.hpp>
typedef boost::property<boost::edge_weight_t, double> EdgeWeight;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property, EdgeWeight> DirectedGraph;
int main() {

    DirectedGraph g;
    /**
    * Create the graph drawn below.
    *
    *
    *      10
    *   ________
    *  / 1   3  \
    *  1-->2--->3
        ^  ^  /
    *   8\0| /2
    *     \|/
    *      4
    **/
    boost::add_edge(0, 1, 1, g);
    boost::add_edge(1, 2, 3, g);
    boost::add_edge(2, 3, 2, g);
    boost::add_edge(3, 1, 0, g);
    boost::add_edge(3, 0, 8, g);
    boost::add_edge(0, 2, 10, g);
    std::cout<<boost::karp_minimum_cycle_mean(g);
}
