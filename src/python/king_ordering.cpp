// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/king_ordering.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template<typename Graph>
boost::python::list
king_ordering(const Graph& g)
{
  std::list<typename Graph::Vertex> ordering;
  boost::python::list result;
  boost::king_ordering(g, std::back_inserter(ordering),
                       g.get_vertex_index_map());
  for (typename std::list<typename Graph::Vertex>::iterator i 
         = ordering.begin(); i != ordering.end(); ++i)
    result.append(*i);
  return result;
}

void export_king_ordering()
{
  using boost::python::arg;
  using boost::python::def;
  def("king_ordering", &king_ordering<Graph>, arg("graph"));
}

} } } // end namespace boost::graph::python
