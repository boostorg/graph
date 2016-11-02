//=======================================================================
// Copyright 2001 University of Notre Dame.
// Author: Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/* 
   Sample output:

   filtered edge set: (A,B) (C,D) (D,B) 
   filtered out-edges:
   A --> B 
   B --> 
   C --> D 
   D --> B 
   E --> 
 */

#include <boost/config.hpp>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/graph_utility.hpp>
#include "range_pair.hpp"

template <typename EdgeWeightMap>
struct positive_edge_weight {
  positive_edge_weight() { }
  positive_edge_weight(EdgeWeightMap weight) : m_weight(weight) { }
  template <typename Edge>
  bool operator()(const Edge& e) const {
    return 0 < boost::get(m_weight, e);
  }
  EdgeWeightMap m_weight;
};


int main()
{
  using namespace boost;
  
  using Graph = adjacency_list<multisetS, vecS, directedS,
    no_property, property<edge_weight_t, int>>;
  using EdgeWeightMap = property_map<Graph, edge_weight_t>::type;

  enum { A, B, C, D, E, N };
  const char* name = "ABCDE";
  Graph g(N);
  add_edge(A, B, 2, g);
  add_edge(A, C, 0, g);
  add_edge(C, D, 1, g);
  add_edge(C, D, 0, g);
  add_edge(C, D, 3, g);
  add_edge(C, E, 0, g);
  add_edge(D, B, 3, g);
  add_edge(E, C, 0, g);

  auto weight = get(edge_weight, g);

  std::cout << "unfiltered edge_range(C,D)\n";
  for (const auto& edge : make_range_pair(edge_range(C, D, g)))
    std::cout << name[source(edge, g)] << " --" << weight[edge]
              << "-> " <<  name[target(edge, g)] << "\n";

  positive_edge_weight<EdgeWeightMap> filter(weight);
  using FGraph = filtered_graph<Graph, positive_edge_weight<EdgeWeightMap>>;
  FGraph fg(g, filter);

  std::cout << "filtered edge_range(C,D)\n";
  for (const auto& edge : make_range_pair(edge_range(C, D, fg)))
    std::cout << name[source(edge, fg)] << " --" << weight[edge]
              << "-> " <<  name[target(edge, fg)] << "\n";
  
  return 0;
}
