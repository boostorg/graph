// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Ben Martin
//           Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/minimum_degree_ordering.hpp>
//#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <vector.h>
#include <iostream.h>

namespace boost { namespace graph { namespace python {

  template<typename GraphT>
  boost::python::list 
  minimum_degree_ordering
  (GraphT& g,
   const vector_property_map<int, typename GraphT::VertexIndexMap>* outdegree,
   const vector_property_map<int, typename GraphT::VertexIndexMap>* supernode_size,
   const int delta,
   const typename GraphT::VertexIndexMap *id)
{
  typedef std::vector<int> InversePermutationMap;
  typedef std::vector<int> PermutationMap;
  typedef vector_property_map<int, typename GraphT::VertexIndexMap> OutDegreeMap;
  typedef vector_property_map<int, typename GraphT::VertexIndexMap> SuperNodeSizeMap;
  typedef typename GraphT::VertexIndexMap VertexIndexMap;

  int n = num_vertices(g);

  OutDegreeMap out_degree_map = outdegree ? *outdegree : g. template get_vertex_map<int>("outdegree");
  InversePermutationMap inv_perm_map(n, 0);
  PermutationMap perm_map(n, 0);
  SuperNodeSizeMap sns_map = 
      supernode_size ? *supernode_size : g. template get_vertex_map<int>("supernode_size");
  VertexIndexMap vertex_index_map = id ? *id : g.get_vertex_index_map();

  typedef typename GraphT::vertex_iterator VertexIterator;
  VertexIterator v, vs, ve;
  using std::pair;
  std::pair<VertexIterator, VertexIterator> p;
  p = vertices(g);
  vs = p.first;
  ve = p.second;
  for (v = vs; v != ve; ++v)
    sns_map[*v] = 1;

  boost::minimum_degree_ordering<GraphT, OutDegreeMap, 
                                 InversePermutationMap, PermutationMap,
                                 SuperNodeSizeMap, VertexIndexMap>
                                (g,
                                 out_degree_map,
				 inv_perm_map,
				 perm_map,
				 sns_map,
				 delta,
				 vertex_index_map);

  // Build the new ordering
  typedef typename graph_traits<GraphT>::vertex_descriptor vertex_descriptor;
  std::vector<vertex_descriptor> new_ordering(num_vertices(g));
  vs = p.first;
  ve = p.second;
  for (v = vs; v != ve; ++v)
    new_ordering[perm_map[get(vertex_index_map, *v)]] = *v;

  // Build resulting list for Python
  typedef typename graph_traits<GraphT>::vertices_size_type vertices_size_type;
  boost::python::list result;
  typedef typename std::vector<vertex_descriptor>::iterator iterator;
  for (iterator i = new_ordering.begin(); i != new_ordering.end(); ++i)
    result.append(*i);

  return result;
}

void export_minimum_degree_ordering()
{
  using boost::python::arg;
  using boost::python::def;

  def("minimum_degree_ordering", 
      &minimum_degree_ordering<Digraph>,
      (arg("graph"), 
       arg("out_degree") = (vector_property_map<int, Digraph::VertexIndexMap>*)0,
       arg("supernode_size") = 
         (vector_property_map<int, Digraph::VertexIndexMap>*)0,
       arg("delta") = 0,
       arg("id") = (Digraph::VertexIndexMap*)0));
}

} } } // end namespace boost::graph::python
