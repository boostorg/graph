// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include <list>
#include <iterator>

namespace boost { namespace graph { namespace python {

template<typename Graph>
boost::python::list 
kruskal_minimum_spanning_tree
  (Graph& g,
   const vector_property_map<double, typename Graph::EdgeIndexMap>* in_weight)
{
  typedef vector_property_map<double, typename Graph::EdgeIndexMap> WeightMap;

  WeightMap weight = 
    in_weight? *in_weight : g.template get_edge_map<double>("weight");

  std::list<typename Graph::Edge> mst_edges;
  boost::kruskal_minimum_spanning_tree
    (g, 
     std::back_inserter(mst_edges),
     vertex_index_map(g.get_vertex_index_map()).
     weight_map(weight));
  boost::python::list result;
  for (typename std::list<typename Graph::Edge>::iterator i 
         = mst_edges.begin(); i != mst_edges.end(); ++i)
    result.append(*i);
  return result;
}

void export_kruskal_minimum_spanning_tree()
{
  using boost::python::arg;
  using boost::python::def;

  def("kruskal_minimum_spanning_tree", &kruskal_minimum_spanning_tree<Graph>,
      (arg("graph"), 
       arg("weight_map") = 
         (vector_property_map<double, Graph::EdgeIndexMap>*)0));
}

} } } // end namespace boost::graph::python
