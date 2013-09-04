//=======================================================================
// Copyright 2013 University of Warsaw.
// Authors: Piotr Wygocki 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#define BOOST_TEST_MODULE cycle_canceling_test

#include <boost/test/unit_test.hpp>

#include <boost/graph/cycle_canceling.hpp>
#include <boost/graph/edmonds_karp_max_flow.hpp>

#include "min_cost_max_flow_utils.hpp"


BOOST_AUTO_TEST_CASE(cycle_canceling_def_test) {
    unsigned s,t;
    boost::SampleGraph::Graph g 
        = boost::SampleGraph::getSampleGraph(s, t);

    boost::edmonds_karp_max_flow(g, s, t);
    boost::cycle_canceling(g);

    int cost = boost::find_flow_cost(g);
    BOOST_CHECK_EQUAL(cost, 29);
}

BOOST_AUTO_TEST_CASE(path_augmentation_def_test2) {
    unsigned s,t;
    boost::SampleGraph::Graph g 
        = boost::SampleGraph::getSampleGraph2(s, t);

    boost::edmonds_karp_max_flow(g, s, t);
    boost::cycle_canceling(g);

    int cost =  boost::find_flow_cost(g);
    BOOST_CHECK_EQUAL(cost, 7);
}

BOOST_AUTO_TEST_CASE(cycle_canceling_test) {
    unsigned s,t;
    typedef boost::SampleGraph::Graph Graph;
    Graph g = boost::SampleGraph::getSampleGraph(s, t);

    int N = num_vertices(g);
    std::vector<int> dist(N);
    typedef boost::graph_traits<Graph>::edge_descriptor edge_descriptor;
    std::vector<edge_descriptor> pred(N);

    boost::edmonds_karp_max_flow(g, s, t);
    boost::cycle_canceling(g, boost::distance_map(&dist[0]).predecessor_map(&pred[0]).vertex_index_map(boost::identity_property_map()));

    int cost = boost::find_flow_cost(g);
    BOOST_CHECK_EQUAL(cost, 29);
}

