// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "basic_graph.hpp"
#include <boost/graph/graphviz.hpp>
#include <boost/python.hpp>
#include <fstream>
#include <string>

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

  if (has_vertex_map(node_id))
    boost::write_graphviz(out, *this, dp, node_id, 
                          get_vertex_map<std::string>(node_id));
  else
    boost::write_graphviz(out, *this, dp, node_id, get_vertex_index_map());
}

template<typename E>
class translate_exception
{
  explicit translate_exception(boost::python::object type) : type(type) { }

public:
  template<typename Base>
  static void declare(const char* name)
  {
    using boost::python::class_;
    using boost::python::bases;

    declare(class_<E, bases<Base> >(name));
  }

  static void declare(boost::python::object type)
  {
    using boost::python::register_exception_translator;
    register_exception_translator<E>(translate_exception(type));
  }

  void operator()(const E& e) const
  {
    using boost::python::object;
    PyErr_SetObject(type.ptr(), object(e).ptr());
  }

private:
  boost::python::object type;
};

void export_graphviz()
{
  using boost::python::class_;
  using boost::python::bases;
  using boost::python::init;
  using boost::python::no_init;
  using boost::python::object;

  object ge_type = 
    class_<graph_exception, noncopyable >("graph_exception", no_init);
  translate_exception<graph_exception>::declare(ge_type);

  object bpe_type = 
    class_<bad_parallel_edge, bases<graph_exception> >("bad_parallel_edge", 
                                                       no_init)
      .def(init<std::string, std::string>());
  translate_exception<bad_parallel_edge>::declare(bpe_type);

  translate_exception<directed_graph_error>
    ::declare<graph_exception>("directed_graph_error");
  translate_exception<undirected_graph_error>
    ::declare<graph_exception>("undirected_graph_error");
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
