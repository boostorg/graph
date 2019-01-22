//=======================================================================
// Copyright 2012
// Authors: David Doria
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/directed_graph.hpp> // A subclass to provide reasonable arguments to adjacency_list for a typical directed graph

int main(int,char*[])
{
  // directed_graph is a subclass of adjacency_list which gives you object oriented access to functions
  // like add_vertex and add_edge, which makes the code easier to understand. However, it hard codes many
  // of the template parameters, so it is much less flexible.

  using Graph = boost::directed_graph<>;
  Graph g;
  auto v0 = g.add_vertex();
  auto v1 = g.add_vertex();

  g.add_edge(v0, v1);

  return 0;
}
