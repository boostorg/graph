#include <boost/graph/successive_shortest_path.hpp>
#include <boost/graph/find_flow_cost.hpp>

#include "../test/min_cost_max_flow_utils.hpp"


int main() {
    unsigned s,t;
    boost::SampleGraph::Graph g 
        = boost::SampleGraph::getSampleGraph(s, t);

    boost::successive_shortest_path(g, s, t);

    int cost =  boost::find_flow_cost(g);
    assert(cost == 29);

    return 0;
}



