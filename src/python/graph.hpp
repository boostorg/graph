// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_GRAPH_HPP
#define BOOST_GRAPH_PYTHON_GRAPH_HPP

#include "basic_graph.hpp"

namespace boost { namespace graph { namespace python {

  typedef basic_graph<undirectedS> Graph;

#if 0
  class Graph : public basic_graph<undirectedS>
  {
    typedef basic_graph<undirectedS> inherited;

  public:
    Graph() : inherited() { }
    Graph(const std::string& filename, graph_file_kind kind)
      : inherited(filename, kind) { }

    vertex_iterator vertices_begin() const;
    vertex_iterator vertices_end() const;
    edge_iterator edges_begin() const;
    edge_iterator edges_end() const;
  }; 
#endif
} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_GRAPH_HPP
