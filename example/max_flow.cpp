//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee
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

#include <boost/config.hpp>
#include <iostream>
#include <string>
#include <boost/graph/maximum_flow.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/read_dimacs.hpp>

int
main()
{
  using namespace boost;

  typedef adjacency_list_traits<vecS, vecS, directedS> Traits;
  typedef adjacency_list<vecS, vecS, directedS, 
    property<vertex_name_t, std::string>,
    property<edge_capacity_t, long,
      property<edge_residual_capacity_t, long,
        property<edge_reverse_t, Traits::edge_descriptor> > >
  > Graph;

  Graph g;
  
  long flow;

  property_map<Graph, edge_capacity_t>::type 
    capacity = get(edge_capacity, g);

  property_map<Graph, edge_residual_capacity_t>::type 
    residual_capacity = get(edge_residual_capacity, g);

  property_map<Graph, edge_reverse_t>::type 
    reverse_edge = get(edge_reverse, g);

  Traits::vertex_descriptor s, t;

  read_dimacs_max_flow(g, capacity, reverse_edge, s, t);

  print_graph(g);
  
  flow = maximum_flow(g, s, t, capacity, get(edge_residual_capacity, g),
                      get(edge_reverse, g), get(vertex_index, g));
  
  std::cout << "flow: " << flow << std::endl;
  
  return 0;
}
