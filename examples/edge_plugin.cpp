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
//
//  Sample output:
//
#include <iostream>

#include <boost/utility.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>


using namespace boost;
using namespace std;


struct edge_flow { 
  typedef boost::edge_property_tag kind; 
  enum { num = 200 };  // for partial spec. workaround
};
struct edge_capacity { 
  typedef boost::edge_property_tag kind; 
  enum { num = 201 }; // for partial spec. workaround
};

template <class Graph, class Capacity, class Flow>
void print_network(Graph& G, Capacity capacity, Flow flow)
{
  typedef typename boost::graph_traits<Graph>::vertex_iterator    Viter;
  typedef typename boost::graph_traits<Graph>::out_edge_iterator OutEdgeIter;
  typedef typename boost::graph_traits<Graph>::in_edge_iterator InEdgeIter;

  Viter ui, uiend;
  boost::tie(ui, uiend) = vertices(G);

  for (; ui != uiend; ++ui) {
    OutEdgeIter out, out_end;
    cout << *ui << "\t";

    boost::tie(out, out_end) = out_edges(*ui, G);
    for(; out != out_end; ++out)
      cout << "--(" << capacity[*out] << ", " << flow[*out] << ")--> "
           << target(*out,G) << "\t";

    InEdgeIter in, in_end;
    cout << endl << "\t";
    boost::tie(in, in_end) = in_edges(*ui, G);
    for(; in != in_end; ++in)
      cout << "<--(" << capacity[*in] << "," << flow[*in] << ")-- "
           << source(*in,G) << "\t";

    cout << endl;
  }
}


int main(int argc, char* argv[])
{
  typedef plugin<edge_capacity, int> Cap;
  typedef plugin<edge_flow, int, Cap> Flow;
  typedef adjacency_list<vecS, vecS, bidirectionalS, 
     no_plugin, Flow> Graph;

  const int num_vertices = 9;
  Graph G(num_vertices);

  /*          2<----5 
             /       ^
            /         \
           V           \ 
    0 ---->1---->3----->6--->8
           \           ^
            \         /
             V       /
             4----->7
   */

  add_edge(G, 0, 1, Flow(10, Cap(8)));

  add_edge(G, 1, 4, Flow(20, Cap(12)));
  add_edge(G, 4, 7, Flow(20, Cap(12)));
  add_edge(G, 7, 6, Flow(20, Cap(12)));

  add_edge(G, 1, 3, Flow(40, Cap(12)));
  add_edge(G, 3, 6, Flow(40, Cap(12)));

  add_edge(G, 6, 5, Flow(20, Cap(16)));
  add_edge(G, 5, 2, Flow(20, Cap(16)));
  add_edge(G, 2, 1, Flow(20, Cap(16)));

  add_edge(G, 6, 8, Flow(10, Cap(8)));

  typedef boost::graph_traits<Graph>::edge_descriptor Edge;

  property_map<Graph, edge_capacity>::type capacity = get(edge_capacity(), G);
  property_map<Graph, edge_flow>::type flow = get(edge_flow(), G);

  print_network(G, capacity, flow);

  boost::graph_traits<Graph>::vertex_iterator v, v_end;
  boost::graph_traits<Graph>::out_edge_iterator e, e_end;
  int f = 0;
  for (tie(v, v_end) = vertices(G); v != v_end; ++v)
    for (tie(e, e_end) = out_edges(*v, G); e != e_end; ++e)
      flow[*e] = ++f;
  cout << endl << endl;

  remove_edge(G, 6, 8);

  print_network(G, capacity, flow);

          
  return 0;
}
