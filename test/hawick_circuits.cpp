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
    template < typename Graph, typename Visitor >
    void operator()(Graph const& g, Visitor const& v) const
    {
        boost::hawick_circuits(g, v);
    }
};

struct call_hawick_unique_circuits
{
    template < typename Graph, typename Visitor >
    void operator()(Graph const& g, Visitor const& v) const
    {
        boost::hawick_unique_circuits(g, v);
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
    std::cout << "---------hawick_circuits---------\n";
    cycle_test(call_hawick_circuits());

    std::cout << "\n\n---------hawick_unique_circuits---------\n";
    cycle_test(call_hawick_unique_circuits());

    // Make sure we can pass a reference_wrapper to the algorithm.
#if __cplusplus >= 201103L
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
#endif // >= C++11
}
