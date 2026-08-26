// Copyright Arnaud Becheler 2026.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or the copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Shared helpers for the maximum_adjacency_search and stoer_wagner_min_cut
// characterization tests: a curated set of small graphs each with its known
// global min cut, plus a push_relabel_max_flow oracle used to cross-check the
// MAS legal ordering property.

#ifndef BOOST_GRAPH_TEST_MAS_SW_MAXFLOW_ORACLE_HPP
#define BOOST_GRAPH_TEST_MAS_SW_MAXFLOW_ORACLE_HPP

#include <initializer_list>
#include <vector>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/push_relabel_max_flow.hpp>
#include <boost/property_map/property_map.hpp>

namespace mas_sw_oracle
{

using undirected_graph = boost::adjacency_list<
    boost::vecS,
    boost::vecS,
    boost::undirectedS,
    boost::no_property,
    boost::property< boost::edge_weight_t, int >
>;

using weight_map_type = boost::property_map< undirected_graph, boost::edge_weight_t >::type;
using weight_type = boost::property_traits< weight_map_type >::value_type;
using vertex_descriptor = boost::graph_traits< undirected_graph >::vertex_descriptor;
using edge_descriptor = boost::graph_traits< undirected_graph >::edge_descriptor;

// Directed network used only by the max-flow oracle.
using flow_traits = boost::adjacency_list_traits< boost::vecS, boost::vecS, boost::directedS >;
using flow_graph = boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS,
    boost::no_property,
    boost::property< boost::edge_capacity_t, weight_type,
        boost::property< boost::edge_residual_capacity_t, weight_type,
            boost::property< boost::edge_reverse_t, flow_traits::edge_descriptor > > > >;

// Minimum u-v cut of the undirected graph via max-flow.
inline weight_type undirected_min_cut(const undirected_graph& g, vertex_descriptor u, vertex_descriptor v)
{
    flow_graph fg(num_vertices(g));
    auto capacity_map = get(boost::edge_capacity, fg);
    auto reverse_edge_map = get(boost::edge_reverse, fg);
    auto weight_map = get(boost::edge_weight, g);

    boost::graph_traits< undirected_graph >::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    {
        // read the undirected edge and its weight
        const vertex_descriptor a = source(*ei, g);
        const vertex_descriptor b = target(*ei, g);
        const weight_type w = get(weight_map, *ei);
        // arc a to b at capacity w, paired with a zero capacity reverse
        const auto a_to_b = add_edge(a, b, fg).first;
        const auto a_to_b_reverse = add_edge(b, a, fg).first;
        put(capacity_map, a_to_b, w);
        put(capacity_map, a_to_b_reverse, 0);
        put(reverse_edge_map, a_to_b, a_to_b_reverse);
        put(reverse_edge_map, a_to_b_reverse, a_to_b);
        // opposite arc b to a, its own capacity w and zero capacity reverse
        const auto b_to_a = add_edge(b, a, fg).first;
        const auto b_to_a_reverse = add_edge(a, b, fg).first;
        put(capacity_map, b_to_a, w);
        put(capacity_map, b_to_a_reverse, 0);
        put(reverse_edge_map, b_to_a, b_to_a_reverse);
        put(reverse_edge_map, b_to_a_reverse, b_to_a);
    }
    return boost::push_relabel_max_flow(fg, u, v);
}

inline weight_type weighted_degree(const undirected_graph& g, vertex_descriptor v)
{
    auto weight_map = get(boost::edge_weight, g);
    weight_type sum = 0;
    boost::graph_traits< undirected_graph >::out_edge_iterator oi, oi_end;
    for (boost::tie(oi, oi_end) = out_edges(v, g); oi != oi_end; ++oi) sum += get(weight_map, *oi);
    return sum;
}

struct weighted_edge
{
    std::size_t u;
    std::size_t v;
    weight_type w;
};

// Build an undirected weighted graph from an explicit edge list.
inline undirected_graph make_weighted_graph(std::size_t n, std::initializer_list< weighted_edge > edges)
{
    undirected_graph g(n);
    auto weight_map = get(boost::edge_weight, g);
    for (const weighted_edge& e : edges)
    {
        const auto ed = add_edge(e.u, e.v, g).first;
        put(weight_map, ed, e.w);
    }
    return g;
}

inline bool is_connected(const undirected_graph& g)
{
    std::vector< std::size_t > component(num_vertices(g));
    auto components_map = boost::make_iterator_property_map(component.begin(), get(boost::vertex_index, g));
    return boost::connected_components(g, components_map) == 1U;
}

// A curated graph together with its known global minimum cut.
struct curated_graph
{
    undirected_graph graph;
    weight_type min_cut;
};

// Deterministic set of small graphs, each chosen to trigger a specific
// condition inside maximum_adjacency_search and stoer_wagner_min_cut.
inline std::vector< curated_graph > curated_graphs()
{
    std::vector< curated_graph > graphs;
    // path: every cut is a single edge, min is the lightest
    graphs.push_back({ make_weighted_graph(4, { { 0, 1, 3 }, { 1, 2, 1 }, { 2, 3, 2 } }), 1 });
    // triangle: min cut isolates the lowest weighted degree vertex
    graphs.push_back({ make_weighted_graph(3, { { 0, 1, 2 }, { 0, 2, 6 }, { 1, 2, 7 } }), 8 });
    // two triangles joined by a light bridge (issue 286)
    graphs.push_back({ make_weighted_graph(6, { { 0, 1, 10 }, { 0, 2, 10 }, { 1, 2, 10 }, { 3, 4, 10 }, { 3, 5, 10 }, { 4, 5, 10 }, { 0, 3, 1 } }), 1 });
    // multigraph: parallel edge weights must sum
    graphs.push_back({ make_weighted_graph(3, { { 0, 1, 3 }, { 1, 2, 1 }, { 1, 2, 1 }, { 2, 0, 1 } }), 3 });
    // cycle with equal weights: ties in the reach counts
    graphs.push_back({ make_weighted_graph(4, { { 0, 1, 1 }, { 1, 2, 1 }, { 2, 3, 1 }, { 3, 0, 1 } }), 2 });
    // star with one heavy spoke: min cut isolates the lightest leaf
    graphs.push_back({ make_weighted_graph(5, { { 0, 1, 1 }, { 0, 2, 1 }, { 0, 3, 1 }, { 0, 4, 5 } }), 1 });
    // Stoer Wagner 1997 example
    graphs.push_back({ make_weighted_graph(8, { { 0, 1, 2 }, { 1, 2, 3 }, { 2, 3, 4 }, { 0, 4, 3 }, { 1, 4, 2 }, { 1, 5, 2 }, { 2, 6, 2 }, { 3, 6, 2 }, { 3, 7, 2 }, { 4, 5, 3 }, { 5, 6, 1 }, { 6, 7, 3 } }), 4 });
    return graphs;
}

} // namespace mas_sw_oracle

#endif // BOOST_GRAPH_TEST_MAS_SW_MAXFLOW_ORACLE_HPP
