// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/incremental_components.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

template<typename Graph>
class IncrementalComponents
{
public:
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef vector_property_map<vertex_descriptor, 
                              typename Graph::VertexIndexMap> ParentMap;
  typedef vector_property_map<std::size_t,
                              typename Graph::VertexIndexMap> RankMap;

  IncrementalComponents(const Graph& g)
    : ds(RankMap(num_vertices(g), g.get_vertex_index_map()),
         ParentMap(num_vertices(g), g.get_vertex_index_map()))
  {
    initialize_incremental_components(g, ds);
    incremental_components(g, ds);
  }

  void make_set(vertex_descriptor u) { ds.make_set(u); }

  void union_set(vertex_descriptor u, vertex_descriptor v) 
  { ds.union_set(u, v); }

  bool same_component(vertex_descriptor u, vertex_descriptor v)
  { return boost::same_component(u, v, ds); }
  
private:
  disjoint_sets<RankMap, ParentMap> ds;
};

template<typename Graph>
std::auto_ptr<IncrementalComponents<Graph> >
incremental_components(Graph& g)
{ 
  typedef std::auto_ptr<IncrementalComponents<Graph> > result_type;
  return result_type(new IncrementalComponents<Graph>(g)); 
}

void export_incremental_components()
{
  using boost::python::arg;
  using boost::python::def;
  using boost::python::class_;
  using boost::python::no_init;
  class_<IncrementalComponents<Graph> >("IncrementalComponents", no_init)
    .def("make_set", &IncrementalComponents<Graph>::make_set)
    .def("union_set", &IncrementalComponents<Graph>::union_set)
    .def("same_component", &IncrementalComponents<Graph>::same_component)
    ;
  def("incremental_components", &incremental_components<Graph>);
}

} } } // end namespace boost::graph::python
