// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "graph.hpp"
#include "basic_graph.cpp"
#include <iostream>

namespace boost { namespace graph { namespace python {

void export_Graph()
{
  export_basic_graph<undirectedS>("Graph");
}
} } } // end namespace boost::graph::python
