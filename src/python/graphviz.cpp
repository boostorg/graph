#include "basic_graph.hpp"
#include <boost/graph/graphviz.hpp>
#include <fstream>

namespace boost { namespace graph { namespace python {

template<typename DirectedS>
void 
basic_graph<DirectedS>::read_graphviz(const std::string& filename, 
                                      const std::string& node_id)
{
  std::ifstream in(filename.c_str());
  boost::read_graphviz(in, *this, dp, node_id);
}

template<typename DirectedS>
void 
basic_graph<DirectedS>::write_graphviz(const std::string& filename, 
                                       const std::string& node_id)
{
  std::ofstream out(filename.c_str());
  boost::write_graphviz(out, *this, dp, node_id, 
                        get_vertex_map<std::string>("node_id"));
}

// Explicit instantiations
template 
  void 
   basic_graph<undirectedS>::read_graphviz(const std::string& filename,
                                           const std::string& node_id);
template 
  void 
   basic_graph<undirectedS>::write_graphviz(const std::string& filename,
                                            const std::string& node_id);

template 
  void 
   basic_graph<bidirectionalS>::read_graphviz(const std::string& filename,
                                              const std::string& node_id);
template 
  void 
   basic_graph<bidirectionalS>::write_graphviz(const std::string& filename,
                                               const std::string& node_id);

} } } // end namespace boost::graph::python
