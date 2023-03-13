// (C) Copyright 2013 Louis Dionne
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include "cycle_test.hpp"
#include <boost/graph/hawick_circuits.hpp>
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

int main()
{
    // The last two arguments to cycle_test() are the expected (correct)
    // number of circuits in the undirected and directed test graphs.

    std::cout << "---------hawick_circuits---------\n";
    cycle_test(call_hawick_circuits(), 30, 31);

    std::cout << "\n\n---------hawick_unique_circuits---------\n";
    cycle_test(call_hawick_unique_circuits(), 27, 24);

    std::cout << "\n\n---------hawick_circuits(max_length = 4)---------\n";
    cycle_test(call_hawick_circuits(4), 28, 13);

    std::cout << "\n\n---------hawick_unique_circuits(max_length = 4)---------\n";
    cycle_test(call_hawick_unique_circuits(4), 25, 10);

    std::cout << "\n\n";
    return boost::report_errors();
}
