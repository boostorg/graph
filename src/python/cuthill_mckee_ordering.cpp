// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template <typename Graph>
class out_degree_property_map 
  : public put_get_helper<typename graph_traits<Graph>::degree_size_type,
                          out_degree_property_map<Graph> >                  
{
public:
  typedef typename graph_traits<Graph>::vertex_descriptor key_type;
  typedef typename graph_traits<Graph>::degree_size_type value_type;
  typedef value_type reference;
  typedef readable_property_map_tag category;
  out_degree_property_map(const Graph& g) : m_g(g) { }
  value_type operator[](const key_type& v) const {
    return out_degree(v, m_g);
  }
private:
  const Graph& m_g;
};
template <typename Graph>
inline out_degree_property_map<Graph>
make_out_degree_map(const Graph& g) {
  return out_degree_property_map<Graph>(g);
}


template<typename Graph>
boost::python::list
cuthill_mckee_ordering
  (const Graph& g,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  std::list<typename Graph::Vertex> ordering;
  boost::python::list result;
  boost::cuthill_mckee_ordering(g, std::back_inserter(ordering), 
                                color, make_out_degree_map(g));
  for (typename std::list<typename Graph::Vertex>::iterator i 
         = ordering.begin(); i != ordering.end(); ++i)
    result.append(*i);
  return result;
}

void export_cuthill_mckee_ordering()
{
  using boost::python::arg;
  using boost::python::def;

  def("cuthill_mckee_ordering", &cuthill_mckee_ordering<Graph>, 
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));
  def("cuthill_mckee_ordering", &cuthill_mckee_ordering<Digraph>, 
      (arg("graph"),
       arg("color_map") = 
         (vector_property_map<default_color_type, Digraph::VertexIndexMap>*)0));
}

} } } // end namespace boost::graph::python
