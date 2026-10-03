// Copyright (C) 2026 Arnaud Becheler
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// Round-trips a graph through write_graphviz_dp and read_graphviz. The vertex
// and edge string values are chosen to straddle the DOT quoting boundary in
// escape_dot_string, so a writer that mis-quotes produces DOT that re-reads
// into different values and fails the comparison.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/core/lightweight_test.hpp>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace
{

struct vertex_props
{
    std::string node_id;
    std::string label;
};

struct edge_props
{
    std::string elabel;
};

using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
    boost::directedS, vertex_props, edge_props >;

// First four are valid unquoted DOT ids/numerals
// the rest force quoting or escaping: space, embedded quote, digit-led
// identifier, multi-dot numeral, a lone dash, and a leading non-id character.
const std::vector< std::string >& boundary_labels()
{
    static const std::vector< std::string > labels = { "plain", "id_123", "42",
        "-3.14", "has space", "has\"quote", "9lives", "1.2.3", "-", "@home" };
    return labels;
}

graph_t make_graph()
{
    const std::vector< std::string >& labels = boundary_labels();
    graph_t g;
    std::vector< graph_t::vertex_descriptor > added;
    added.reserve(labels.size());
    for (std::size_t i = 0; i < labels.size(); ++i)
    {
        graph_t::vertex_descriptor vd = boost::add_vertex(g);
        g[vd].node_id = "n" + std::to_string(i);
        g[vd].label = labels[i];
        added.push_back(vd);
    }
    for (std::size_t i = 1; i < added.size(); ++i)
    {
        graph_t::edge_descriptor ed
            = boost::add_edge(added[i - 1], added[i], g).first;
        g[ed].elabel = "e " + std::to_string(i); // space forces quoting
    }
    return g;
}

std::map< std::string, std::string > labels_by_id(const graph_t& g)
{
    std::map< std::string, std::string > m;
    graph_t::vertex_iterator it, end;
    for (boost::tie(it, end) = boost::vertices(g); it != end; ++it)
        m[g[*it].node_id] = g[*it].label;
    return m;
}

std::multiset< std::string > edges_repr(const graph_t& g)
{
    std::multiset< std::string > s;
    graph_t::edge_iterator it, end;
    for (boost::tie(it, end) = boost::edges(g); it != end; ++it)
    {
        const std::string src = g[boost::source(*it, g)].node_id;
        const std::string tgt = g[boost::target(*it, g)].node_id;
        s.insert(src + "->" + tgt + ":" + g[*it].elabel);
    }
    return s;
}

boost::dynamic_properties make_properties(graph_t& g)
{
    boost::dynamic_properties dp;
    dp.property("node_id", boost::get(&vertex_props::node_id, g));
    dp.property("label", boost::get(&vertex_props::label, g));
    dp.property("elabel", boost::get(&edge_props::elabel, g));
    return dp;
}

} // namespace

int main()
{
    graph_t g = make_graph();
    boost::dynamic_properties dp = make_properties(g);

    std::ostringstream out;
    boost::write_graphviz_dp(out, g, dp, std::string("node_id"));
    const std::string text = out.str();

    graph_t g2;
    boost::dynamic_properties dp2 = make_properties(g2);
    const bool ok = boost::read_graphviz(text.begin(), text.end(), g2, dp2, "node_id");
    BOOST_TEST(ok);

    BOOST_TEST(boost::num_vertices(g) == boost::num_vertices(g2));
    BOOST_TEST(boost::num_edges(g) == boost::num_edges(g2));
    BOOST_TEST(labels_by_id(g) == labels_by_id(g2));
    BOOST_TEST(edges_repr(g) == edges_repr(g2));

    return boost::report_errors();
}
