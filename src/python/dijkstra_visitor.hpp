// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_DIJKSTRA_VISITOR_HPP
#define BOOST_GRAPH_PYTHON_DIJKSTRA_VISITOR_HPP
namespace boost { namespace graph { namespace python {

#define BGL_PYTHON_VISITOR dijkstra_visitor
#define BGL_PYTHON_EVENTS_HEADER "dijkstra_events.hpp"
#include "visitor.hpp"
#undef BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_VISITOR

} } } // end namespace boost::graph::python
#endif BOOST_GRAPH_PYTHON_DIJKSTRA_VISITOR_HPP
