// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine

BGL_PYTHON_EVENT(initialize_vertex, vertex_descriptor)
BGL_PYTHON_EVENT(start_vertex, vertex_descriptor)
BGL_PYTHON_EVENT(discover_vertex, vertex_descriptor)
BGL_PYTHON_EVENT(examine_vertex, vertex_descriptor)
BGL_PYTHON_EVENT(examine_edge, edge_descriptor)
BGL_PYTHON_EVENT(tree_edge, edge_descriptor)
BGL_PYTHON_EVENT(back_edge, edge_descriptor)
BGL_PYTHON_EVENT(forward_or_cross_edge, edge_descriptor)
BGL_PYTHON_EVENT(finish_vertex, vertex_descriptor)
