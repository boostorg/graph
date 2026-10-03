//  (C) Copyright 2004 Douglas Gregor and Jeremy Siek
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)

// Compile-fail test: remove_edge() must be rejected at compile time when the
// EdgeList of an adjacency_list is vecS. Removing an edge from a vector-backed
// edge list would renumber every stored edge index past the removal point, so
// adjacency_list guards each remove_edge overload with a static assertion
// instead of supporting the operation. This test pins that guard (see
// test/Jamfile.v2, [ compile-fail bidir_vec_remove_edge.cpp ]).

#include <boost/graph/adjacency_list.hpp>

int main()
{
    using graph = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::bidirectionalS, boost::no_property, boost::no_property,
        boost::no_property, boost::vecS >;
    graph g(2);
    add_edge(0, 1, g);
    remove_edge(0, 1, g); // must not compile
}
