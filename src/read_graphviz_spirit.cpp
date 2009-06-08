// Copyright 2004-5 Trustees of Indiana University

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//
// read_graphviz_spirit.hpp - 
//   Initialize a model of the BGL's MutableGraph concept and an associated
//  collection of property maps using a graph expressed in the GraphViz
// DOT Language.  
//
//   Based on the grammar found at:
//   http://www.graphviz.org/cvs/doc/info/lang.html
//
//   See documentation for this code at: 
//     http://www.boost.org/libs/graph/doc/read-graphviz.html
//

// Authors: Ronald Garcia and Douglas Gregor
//

#define BOOST_GRAPH_SOURCE

#ifndef BOOST_GRAPH_READ_GRAPHVIZ_ITERATORS
#  define BOOST_GRAPH_READ_GRAPHVIZ_ITERATORS
#endif
#include <boost/graph/graphviz.hpp>
#include <iostream>

namespace boost { namespace detail { namespace graph { 

#if 0
BOOST_GRAPH_DECL
bool read_graphviz(std::istream& in, mutate_graph& graph) 
{
  using namespace boost;

  typedef std::istream_iterator<char> is_t;

  std::string str((is_t(in)), is_t());

  return read_graphviz(str.begin(), str.end());
}
#endif

} } } // end namespace boost::detail::graph
