#include <boost/graph/cycle_canceling.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>

#include "../test/min_cost_max_flow_utils.hpp"


int main() {
    unsigned s,t;
    boost::SampleGraph::Graph g 
        = boost::SampleGraph::getSampleGraph(s, t);

    boost::edmonds_karp_max_flow(g, s, t);
    boost::cycle_canceling(g);

    int cost = boost::find_flow_cost(g);
    assert(cost == 29);
    return 0;
}

