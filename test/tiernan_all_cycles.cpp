// (C) Copyright 2007-2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/graph/graph_utility.hpp>
#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/directed_graph.hpp>
#include <boost/graph/tiernan_all_cycles.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>

#include <boost/random/linear_congruential.hpp>

#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>

using namespace std;
using namespace boost;

struct cycle_validator
{
    cycle_validator(size_t& c) : cycles(c) {}

    template < typename Path, typename Graph >
    void cycle(const Path& p, const Graph& g)
    {
        ++cycles;
        // Check to make sure that each of the vertices in the path
        // is truly connected and that the back is connected to the
        // front - it's not validating that we find all paths, just
        // that the paths are valid.
        typename Path::const_iterator i, j, last = prior(p.end());
        for (i = p.begin(); i != last; ++i)
        {
            j = boost::next(i);
            BOOST_ASSERT(edge(*i, *j, g).second);
        }
        BOOST_ASSERT(edge(p.back(), p.front(), g).second);
    }

    size_t& cycles;
};

template < typename Graph > void test()
{
    typedef erdos_renyi_iterator< boost::minstd_rand, Graph > er;

    // Generate random graph with N vertices and probability P
    // of edge connection.
    static const size_t N = 20;
    static const double P = 0.1;
    boost::minstd_rand rng(42);

    Graph g(er(rng, N, P), er(), N);
    renumber_indices(g);
    print_edges(g, get(vertex_index, g));

    size_t cycles = 0;
    cycle_validator vis(cycles);
    tiernan_all_cycles(g, vis);
    cout << "# cycles: " << vis.cycles << "\n";
}

// state in a plain data member, so it survives only through std::ref
struct cycle_tally
{
    template < typename Path, typename Graph >
    void cycle(const Path&, const Graph&)
    {
        ++count;
    }
    std::size_t count = 0;
};

void test_stateful_visitor_with_ref()
{
    using graph_t = boost::directed_graph<>;
    graph_t g;
    graph_t::vertex_descriptor v0 = g.add_vertex();
    graph_t::vertex_descriptor v1 = g.add_vertex();
    graph_t::vertex_descriptor v2 = g.add_vertex();
    graph_t::vertex_descriptor v3 = g.add_vertex();

    // a triangle and a two cycle, so two elementary cycles
    g.add_edge(v0, v1);
    g.add_edge(v1, v2);
    g.add_edge(v2, v0);
    g.add_edge(v1, v3);
    g.add_edge(v3, v1);

    cycle_tally tracked;
    boost::tiernan_all_cycles(g, std::ref(tracked));
    BOOST_TEST_EQ(tracked.count, static_cast< std::size_t >(2));

    // by value the caller's visitor is left untouched
    cycle_tally copied;
    boost::tiernan_all_cycles(g, copied);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

int main(int, char*[])
{
    typedef undirected_graph<> Graph;
    typedef directed_graph<> DiGraph;

    std::cout << "*** undirected ***\n";
    test< Graph >();

    std::cout << "*** directed ***\n";
    test< DiGraph >();

    test_stateful_visitor_with_ref();

    return boost::report_errors();
}
