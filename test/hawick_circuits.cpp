// (C) Copyright 2013 Louis Dionne
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include "cycle_test.hpp"
#include <boost/graph/hawick_circuits.hpp>
#include <cstddef>
#include <functional>
#include <iostream>

struct call_hawick_circuits
{
    unsigned int max_length;
    call_hawick_circuits(unsigned int ml = 0) : max_length(ml) {}

    template < typename Graph, typename Visitor >
    void operator()(Graph const& g, Visitor const& v) const
    {
        boost::hawick_circuits(g, v, max_length);
    }
};

struct call_hawick_unique_circuits
{
    unsigned int max_length;
    call_hawick_unique_circuits(unsigned int ml = 0) : max_length(ml) {}

    template < typename Graph, typename Visitor >
    void operator()(Graph const& g, Visitor const& v) const
    {
        boost::hawick_unique_circuits(g, v, max_length);
    }
};

struct not_copyable
{
    not_copyable() { }

    template < typename Path, typename Graph >
    void cycle(Path const&, Graph const&)
    {

    }

private:
    not_copyable(not_copyable const&);
};

int main()
{
    // The last two arguments to cycle_test() are the expected (correct)
    // number of circuits in the undirected and directed test graphs.

    std::cout << "---------hawick_circuits---------\n";
    cycle_test(call_hawick_circuits(), 30, 31);

    std::cout << "\n\n---------hawick_unique_circuits---------\n";
    cycle_test(call_hawick_unique_circuits(), 27, 24);

    // Correct values for max_length = 0 to 10
    // undirected
    std::size_t nc1[] = { 30, 0, 22, 26, 28, 30, 30, 30, 30, 30, 30 };
    // directed
    std::size_t nc2[] = { 31, 0,  3,  7, 13, 17, 22, 24, 27, 30, 31 };
    // undirected, unique
    std::size_t nc3[] = { 27, 0, 19, 23, 25, 27, 27, 27, 27, 27, 27 };
    // directed, unique
    std::size_t nc4[] = { 24, 0,  3,  6, 10, 13, 17, 19, 21, 23, 24 };
    for (unsigned int ml = 0; ml <= 10; ++ml) {
        std::cout << "\n\n---------hawick_circuits(max_length = " << ml;
        std::cout << ")---------\n";
        cycle_test(call_hawick_circuits(ml), nc1[ml], nc2[ml]);

        std::cout << "\n\n---------hawick_unique_circuits(max_length = " << ml;
        std::cout << ")---------\n";
        cycle_test(call_hawick_unique_circuits(ml), nc3[ml], nc4[ml]);
    }

    // Make sure we can pass a reference_wrapper to the algorithm.
    {
        typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS> Graph;
        typedef std::pair<std::size_t, std::size_t> Pair;
        Pair edges[3] = {
            Pair(0, 1), // a->b
            Pair(1, 2), // b->c
            Pair(2, 0), // c->a
        };

        Graph G(3);
        for (int i = 0; i < 3; ++i)
            add_edge(edges[i].first, edges[i].second, G);

        not_copyable visitor;
        boost::hawick_circuits(G, std::ref(visitor));
    }

    std::cout << "\n\n";
    return boost::report_errors();
}
