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

#include <iostream>
#include <string>
#include <boost/graph/maximum_flow.hpp>
#include <boost/graph/adjacency_list.hpp>

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

  const int N = 6;
  Graph g(N);
  
  long flow;

  put(vertex_name, g, 0, std::string("Vancouver"));
  put(vertex_name, g, 1, std::string("Edmonton"));
  put(vertex_name, g, 2, std::string("Calgary"));
  put(vertex_name, g, 3, std::string("Saskatoon"));
  put(vertex_name, g, 4, std::string("Regina"));
  put(vertex_name, g, 5, std::string("Winnipeg"));

  property_map<Graph, edge_capacity_t>::type 
    capacity = get(edge_capacity, g);

  typedef std::pair<int,int> Edge;
  Edge edges[] = { Edge(0, 1), Edge(0, 2),
                   Edge(1, 2), Edge(1, 3),
                   Edge(2, 4),
                   Edge(3, 2), Edge(3, 5),
                   Edge(4, 3), Edge(4, 5) };
  
  for (int i = 0; i < sizeof(edges)/sizeof(Edge); ++i) {
    // Add edges going both directions
    add_edge(edges[i].first, edges[i].second, g);
    graph_traits<Graph>::edge_descriptor e; 
    bool inserted;
    tie(e, inserted) = add_edge(edges[i].second, edges[i].first, g);
    capacity[e] = 0;
  }

  capacity[edge(0,1,g).first] = 16;
  capacity[edge(1,3,g).first] = 12;
  capacity[edge(1,2,g).first] = 10;
  capacity[edge(2,1,g).first] = 4;
  capacity[edge(0,2,g).first] = 13;
  capacity[edge(3,2,g).first] = 9;
  capacity[edge(2,4,g).first] = 14;
  capacity[edge(4,3,g).first] = 7;
  capacity[edge(3,5,g).first] = 20;
  capacity[edge(4,5,g).first] = 4;
  
  Traits::vertex_descriptor s = 0, t = 5;
  
  maximum_flow(g, s, t, capacity, get(edge_residual_capacity, g),
               get(vertex_index, g), flow);
  
  std::cout << "flow: " << flow << std::endl;
  
  return 0;
}
