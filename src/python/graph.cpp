#include "graph.hpp"
#include "basic_graph.cpp"
#include <iostream>

namespace boost { namespace graph { namespace python {

void export_Graph()
{
  export_basic_graph<undirectedS>("Graph");
}
} } } // end namespace boost::graph::python
