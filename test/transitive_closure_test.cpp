// Copyright (C) 2001 Vladimir Prus <ghost@cs.msu.su>
// Copyright (C) 2001 Jeremy Siek <jsiek@cs.indiana.edu>
// Copyright (c) 2026 Arnaud Becheler
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/transitive_closure.hpp>

template < class Graph >
using vertex_iter_t = typename boost::graph_traits< Graph >::vertex_iterator;

template < class Graph >
using out_edge_iter_t = typename boost::graph_traits< Graph >::out_edge_iterator;

template < class Graph >
using adjacency_iter_t = typename boost::graph_traits< Graph >::adjacency_iterator;

template < class Graph >
using vertex_desc_t = typename boost::graph_traits< Graph >::vertex_descriptor;

template < class Graph >
using edge_desc_t = typename boost::graph_traits< Graph >::edge_descriptor;

template < class Graph >
using degree_size_t = typename boost::graph_traits< Graph >::degree_size_type;

template < class Generator >
void generate_graph(int n, double p, std::vector< std::vector< int > >& r1, Generator& uniform01)
{
    r1.clear();
    r1.resize(n);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            if (uniform01() < p)
                r1[i].push_back(j);
}

template < class Graph >
degree_size_t< Graph > num_incident(vertex_desc_t< Graph > u, vertex_desc_t< Graph > v, const Graph& g)
{
    using boost::target; // ADL also finds std::target for pair edges
    degree_size_t< Graph > d = 0;
    out_edge_iter_t< Graph > i, i_end;
    for (boost::tie(i, i_end) = boost::out_edges(u, g); i != i_end; ++i)
        if (target(*i, g) == v)
            ++d;
    return d;
}

// true iff g has a path of at least one edge from u to v. is_reachable treats u
// as reachable from itself, so a cycle through u is detected via its neighbors.
template < typename Graph >
bool has_nontrivial_path(Graph& g, vertex_desc_t< Graph > u, vertex_desc_t< Graph > v)
{
    auto reaches = [&](vertex_desc_t< Graph > from, vertex_desc_t< Graph > to) {
        std::vector< boost::default_color_type > color(boost::num_vertices(g));
        auto index_map = boost::get(boost::vertex_index, g);
        auto color_map = boost::make_iterator_property_map(color.begin(), index_map);
        return boost::is_reachable(from, to, g, color_map);
    };

    if (u != v)
        return reaches(u, v);

    adjacency_iter_t< Graph > k, k_end;
    for (boost::tie(k, k_end) = boost::adjacent_vertices(u, g); k != k_end; ++k)
        if (reaches(*k, u))
            return true;
    return false;
}

// tc must contain edge (i, j) exactly when g has a nontrivial path from i to j,
// and never as a duplicate (num_incident, not mere presence)
template < typename Graph, typename GraphTC >
bool check_transitive_closure(Graph& g, GraphTC& tc)
{
    vertex_iter_t< Graph > i, i_end;
    for (boost::tie(i, i_end) = boost::vertices(g); i != i_end; ++i)
    {
        vertex_iter_t< Graph > j, j_end;
        for (boost::tie(j, j_end) = boost::vertices(g); j != j_end; ++j)
        {
            degree_size_t< Graph > expected = has_nontrivial_path(g, *i, *j) ? 1 : 0;
            if (num_incident(*i, *j, tc) != expected)
                return false;
        }
    }
    return true;
}

// verifies transitive_closure agrees with is_reachable
// if both are bugged, it would pass
template < class Generator >
bool is_random_graph_closure_correct(int n, double p, Generator& uniform01)
{
    std::vector< std::vector< int > > g1, g1_tc;
    generate_graph(n, p, g1, uniform01);

    boost::transitive_closure(g1, g1_tc);

    if (check_transitive_closure(g1, g1_tc)){
        return true;
    } else {
        std::cout << "failing graph: ";
        boost::print_graph(g1, boost::get(boost::vertex_index, g1));
        std::cout << "computed closure: ";
        boost::print_graph(g1_tc, boost::get(boost::vertex_index, g1_tc));
        return false;
    }
}

// closure of a fixed graph checked against a hand-computed edge set
// oracle independent of is_reachable
bool is_fixed_graph_closure_correct()
{
    using graph_t = boost::adjacency_list<>;
    graph_t g(5), g_tc;
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 0, g);
    boost::add_edge(1, 2, g);
    boost::add_edge(1, 4, g);
    boost::add_edge(3, 0, g);
    boost::add_edge(3, 2, g);
    boost::add_edge(4, 2, g);
    boost::add_edge(4, 3, g);

    boost::transitive_closure(g, g_tc);

    constexpr std::pair< int, int > expected[]
        = { { 0, 2 }, { 1, 0 }, { 1, 2 }, { 1, 3 }, { 1, 4 }, { 3, 0 },
              { 3, 2 }, { 4, 0 }, { 4, 2 }, { 4, 3 } };

    if (boost::num_edges(g_tc) != sizeof(expected) / sizeof(expected[0]))
        return false;
    for (const std::pair< int, int >& e : expected)
        if (!boost::edge(e.first, e.second, g_tc).second)
            return false;
    return true;
}

int main()
{
    std::mt19937 engine(42);
    std::uniform_real_distribution< double > dist(0.0, 1.0);
    auto uniform01 = [&] { return dist(engine); };

    for (std::size_t i = 0; i < 100; ++i)
    {
        int n = 0 + int(20 * uniform01());
        double p = uniform01();
        BOOST_TEST(is_random_graph_closure_correct(n, p, uniform01));
    }

    BOOST_TEST(is_fixed_graph_closure_correct());

    return boost::report_errors();
}
