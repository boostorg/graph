// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/fruchterman_reingold.hpp>
#include "graph.hpp"
#include "digraph.hpp"

namespace boost { namespace graph { namespace python {

template<typename Graph>
void 
fruchterman_reingold_force_directed_layout
  (Graph& g,
   const vector_property_map<point2d, typename Graph::VertexIndexMap>* in_pos,
   double width, double height,
   )
{
}

} } } // end namespace boost::graph::python
