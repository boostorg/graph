#include <iostream>
#include <boost/graph/karp_minimum_cycle_mean.hpp>
#include <boost/core/lightweight_test.hpp>
typedef boost::property<boost::edge_weight_t, double> EdgeWeight;
typedef boost::adjacency_list<boost::listS, boost::vecS, boost::directedS, boost::no_property, EdgeWeight> DirectedGraph;
int main() {
    const double epsilon = 0.005;
    {
        DirectedGraph g1;
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
        boost::add_edge(0, 1, 1, g1);
        boost::add_edge(1, 2, 3, g1);
        boost::add_edge(2, 3, 2, g1);
        boost::add_edge(3, 1, 0, g1);
        boost::add_edge(3, 0, 8, g1);
        boost::add_edge(0, 2, 10, g1);
        double min_cycle_mean = boost::karp_minimum_cycle_mean(g1);
        std::cout << min_cycle_mean << std::endl;
        BOOST_TEST(std::abs(min_cycle_mean - 1.666666666) < epsilon);
    }
    {
        DirectedGraph g2;
        /**
        * Create the graph drawn below.
        *      1
        *    1-->2
        *
        **/
        boost::add_edge(0, 1, 1, g2);
        double min_cycle_mean = boost::karp_minimum_cycle_mean(g2);
        std::cout << min_cycle_mean << std::endl;
        BOOST_TEST(std::abs(min_cycle_mean +1) < epsilon);
    }

    return boost::report_errors();
}
