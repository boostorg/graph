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
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_accessor.hpp>
#include <boost/utility.hpp>

/*
  This example demonstrates the differences between directed and
  undirected graphs.

  Sample Output
  
  in a directed graph is (u,v) == (v,u) ? 0
  weight[(u,v)] = 1.2
  weight[(v,u)] = 2.4
  in an undirected graph is (u,v) == (v,u) ? 1
  weight[(u,v)] = 3.1
  weight[(v,u)] = 3.1
  the edges incident to v: (0,1)
  

 */

int
main(int, char*[])
{
  using namespace boost;
  using namespace std;

  const int V = 2;

  typedef plugin<edge_weight, float> Weight;
  typedef adjacency_list<vecS, vecS, undirectedS,
                        no_plugin, Weight> UndirectedGraph;
  UndirectedGraph undigraph(V);

  typedef adjacency_list<vecS, vecS, directedS,
                         no_plugin, Weight> DirectedGraph;
  DirectedGraph digraph(V);


  {
    graph_traits<DirectedGraph>::vertex_descriptor u, v;
    u = vertex(0, digraph);
    v = vertex(1, digraph);
    add_edge(digraph, u, v, Weight(1.2));
    add_edge(digraph, v, u, Weight(2.4));
    graph_traits<DirectedGraph>::edge_descriptor e1, e2;
    bool found;
    tie(e1,found) = edge(u, v, digraph);
    tie(e2,found) = edge(v, u, digraph);
    cout << "in a directed graph is ";
    cout << "(u,v) == (v,u) ? " << (e1 == e2) << endl;

    property_map<DirectedGraph, edge_weight>::type
      weight = get(edge_weight(), digraph);
    cout << "weight[(u,v)] = " << get(weight, e1) << endl;
    cout << "weight[(v,u)] = " << get(weight, e2) << endl;
  }
  {
    graph_traits<UndirectedGraph>::vertex_descriptor u, v;
    u = vertex(0, undigraph);
    v = vertex(1, undigraph);
    add_edge(undigraph, u, v, Weight(3.1));
    graph_traits<UndirectedGraph>::edge_descriptor e1, e2;
    bool found;
    tie(e1,found) = edge(u, v, undigraph);
    tie(e2,found) = edge(v, u, undigraph);
    cout << "in an undirected graph is ";
    cout << "(u,v) == (v,u) ? " << (e1 == e2) << endl;

    property_map<UndirectedGraph, edge_weight>::type
      weight = get(edge_weight(), undigraph);
    cout << "weight[(u,v)] = " << get(weight, e1) << endl;
    cout << "weight[(v,u)] = " << get(weight, e2) << endl;
  }

  // Vertices in undirected graphs don't have "out-edges", they have
  // "incident" edges, but we still use the out_edge() function.
  // Similarly, "in" and "out" have no meaning in undirected graphs
  // but we still use source() and target() to access the unordered
  // pair of vertices connected by the edge.
  cout << "the edges incident to v: ";
  graph_traits<UndirectedGraph>::out_edge_iterator e, e_end;
  graph_traits<UndirectedGraph>::vertex_descriptor 
    s = vertex(0, undigraph);
  for (tie(e, e_end) = out_edges(s, undigraph); e != e_end; ++e)
    cout << "(" << source(*e, undigraph) 
              << "," << target(*e, undigraph) << ")" << endl;

  return 0;
}
