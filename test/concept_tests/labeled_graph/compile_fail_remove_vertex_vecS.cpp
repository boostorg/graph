//=======================================================================
// Copyright 2026
// Author: Becheler Arnaud
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// This file must FAIL to compile.
// remove_vertex is not supported when the label map uses vecS storage because 
// erasing from the underlying vector invalidates all label-to-vertex mappings 
// after the erased position.

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/labeled_graph.hpp>

int main()
{
    typedef boost::labeled_graph<
        boost::adjacency_list<boost::listS, boost::listS, boost::directedS>,
        unsigned,
        boost::vecS
    > Graph;

    Graph g;
    g.add_vertex(0u);
    g.remove_vertex(0u);  // should trigger static_assert
}
