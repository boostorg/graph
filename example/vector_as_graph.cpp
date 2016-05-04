//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifdef BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
#error The vector_as_graph.hpp header requires partial specialization
#endif

#include <vector>
#include <list>
#include <iostream> // needed by graph_utility. -Jeremy
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/graph_utility.hpp>

int
main()
{
  enum
  { r, s, t, u, v, w, x, y };
  char name[] = "rstuvwxy";
  using Graph = std::vector < std::list < int > >;
  Graph g = {
    {v}, //r
    {r, r, w}, //s
    {x}, //t
    {t}, //u
    {}, //v
    {t, x}, //w
    {y}, //x
    {u}}; //y
  boost::print_graph(g, name);
  return 0;
}
