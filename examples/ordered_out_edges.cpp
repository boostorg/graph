//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
#include <iostream>
#include <functional>
#include <string>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

template <class StoredEdge>
struct order_by_name
  : public std::binary_function<StoredEdge,StoredEdge,bool> 
{
  bool operator()(const StoredEdge& e1, const StoredEdge& e2) const {
    return e1.get_target() < e2.get_target() || boost::get(boost::edge_name, e1) < boost::get(boost::edge_name, e2);
  }
};

struct by_name {
  template <class StoredEdge>
  struct bind { typedef order_by_name<StoredEdge> type; };
};

namespace boost {
  template <>
  struct parallel_edge_traits<ordered_setS<by_name> > { 
    typedef allow_parallel_edge_tag type; };
}

int
main()
{
  using namespace boost;
  typedef property<edge_name_t, std::string> EdgeProperty;
  typedef adjacency_list<ordered_setS<by_name>, vecS, undirectedS,
    no_property, EdgeProperty> graph_type;
  graph_type g;
  
  add_edge(0, 1, EdgeProperty("joe"), g);
  add_edge(0, 1, EdgeProperty("chandler"), g);
  add_edge(1, 2, EdgeProperty("curly"), g);
  add_edge(1, 3, EdgeProperty("dick"), g);
  add_edge(2, 4, EdgeProperty("tom"), g);
  add_edge(3, 4, EdgeProperty("harry"), g);

  property_map<graph_type, vertex_index_t>::type id = get(vertex_index, g);
  property_map<graph_type, edge_name_t>::type name = get(edge_name, g);

  graph_traits<graph_type>::vertex_iterator i, end;
  graph_traits<graph_type>::out_edge_iterator ei, edge_end;
  for (boost::tie(i, end) = vertices(g); i != end; ++i) {
    cout << id[*i] << " ";
    for (boost::tie(ei, edge_end) = out_edges(*i, g); ei != edge_end; ++ei)
      cout << " --" << name[*ei] << "--> " << id[target(*ei, g)] << "  ";
    cout << endl;
  }
  
  return 0;
}
