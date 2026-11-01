//=======================================================================
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/connected_components.hpp>
#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/property_map/property_map.hpp>
#include <vector>

using Graph = ::boost::adjacency_list< ::boost::vecS, ::boost::vecS, ::boost::undirectedS >;
using Vertex = ::boost::graph_traits< Graph >::vertex_descriptor;

// Hides every vertex.
struct keep_none
{
    bool operator()(Vertex) const { return false; }
};

using FilteredGraph = ::boost::filtered_graph< Graph, ::boost::keep_all, keep_none >;

// Checks that a filtered graph with no visible vertex has no components.
void test_all_vertices_filtered_out()
{
    const Graph base(3);
    const FilteredGraph g(base, ::boost::keep_all(), keep_none());

    std::vector< int > component_vec(::boost::num_vertices(g), -1);
    auto component_map = ::boost::make_iterator_property_map(component_vec.begin(), ::boost::get(::boost::vertex_index, g));
    const int num_components = ::boost::connected_components(g, component_map);

    BOOST_TEST_EQ(num_components, 0);
}

int main()
{
    test_all_vertices_filtered_out();
    return ::boost::report_errors();
}
