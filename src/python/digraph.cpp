#include "digraph.hpp"
#include "basic_graph.cpp"

namespace boost { namespace graph { namespace python {

void export_Digraph()
{
  export_basic_graph<bidirectionalS>("Digraph");
}
} } } // end namespace boost::graph::python
