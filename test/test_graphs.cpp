// (C) Copyright 2007-2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#include <iostream>

#include <boost/graph/undirected_graph.hpp>
#include <boost/graph/directed_graph.hpp>

// TODO: Finish implementing this test module. In theory, this will become a
// generic testing facility for any kind of graph declaration.

using namespace std;
using namespace boost;

struct node
{
    node() : n() { }
    node(int n) : n(n) { }

    bool operator==(node const& x) const    { return n == x.n; }
    bool operator<(node const& x) const     { return n < x.n; }

    int n;
};

struct arc
{
    arc() : n() { }
    arc(int n) : n(n) { }

    bool operator==(arc const& x) const     { return n == x.n; }

    int n;
};

template <typename Graph>
void test()
{
    typedef typename Graph::vertex_descriptor Vertex;
    Graph g;
    BOOST_ASSERT(num_vertices(g) == 0);
}

int main()
{
    test< undirected_graph<node, arc> >();
    test< directed_graph<node, arc> >();
}

