// (C) Copyright 2009 Dmitry Bufistov, Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>

// Test contributed by Dmitry that validates a read-only property map bug
// for bundled properties.
// TODO: Integrate this into a testing framework.

using namespace boost;

struct EdgeProp
{
  double weight;
};

typedef adjacency_list<vecS, vecS, directedS, no_property, EdgeProp > graph_t;
int main()
{
    typedef property_map<graph_t, double EdgeProp::*>::type WeightMap;
    typedef property_map<graph_t, double EdgeProp::*>::const_type cWeightMap;
    typedef graph_traits<graph_t>::edge_descriptor Edge;
    function_requires<ReadablePropertyMapConcept<WeightMap, Edge> >();
    function_requires<ReadablePropertyMapConcept<cWeightMap, Edge> >();
    return 0;
}
