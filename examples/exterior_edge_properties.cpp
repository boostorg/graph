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
// This example is similar to the one in edge_plugin.cpp.
// The only difference is that this example uses exterior
// property storage instead of interior (plugins).
//
//  Sample output:
//
//    0	--(10, 8)--> 1	
//
//    1	--(20, 12)--> 4	--(40, 12)--> 3	
//  	  <--(10,8)-- 0	<--(20,16)-- 2	
//    2	--(20, 16)--> 1	
//  	  <--(20,16)-- 5	
//    3	--(40, 12)--> 6	
//  	  <--(40,12)-- 1	
//    4	--(20, 12)--> 7	
//  	  <--(20,12)-- 1	
//    5	--(20, 16)--> 2	
//  	  <--(20,16)-- 6	
//    6	--(20, 16)--> 5	 --(10, 8)--> 8	
//  	  <--(20,12)-- 7 <--(40,12)-- 3	
//    7	--(20, 12)--> 6	
//  	  <--(20,12)-- 4	
//    8	
//  	  <--(10,8)-- 6	


#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/property_map.hpp>

template <class Graph, class Capacity, class Flow>
void print_network(Graph& G, Capacity capacity, Flow flow)
{
  typedef typename boost::graph_traits<Graph>::vertex_iterator    Viter;
  typedef typename boost::graph_traits<Graph>::out_edge_iterator OutEdgeIter;
  typedef typename boost::graph_traits<Graph>::in_edge_iterator InEdgeIter;

  Viter ui, uiend;
  for (boost::tie(ui, uiend) = boost::vertices(G); ui != uiend; ++ui) {
    OutEdgeIter out, out_end;
    std::cout << *ui << "\t";

    for(boost::tie(out, out_end) = boost::out_edges(*ui, G); out != out_end; ++out)
      std::cout << "--(" << boost::get(capacity, *out) << ", " 
	   << boost::get(flow, *out) << ")--> " << boost::target(*out,G) << "\t";
    std::cout << std::endl << "\t";

    InEdgeIter in, in_end;    
    for(boost::tie(in, in_end) = boost::in_edges(*ui, G); in != in_end; ++in)
      std::cout << "<--(" << boost::get(capacity, *in) << "," << boost::get(flow, *in) << ")-- "
           << boost::source(*in, G) << "\t";
    std::cout << std::endl;
  }
}


int main(int argc, char* argv[]) {

  typedef boost::adjacency_list<boost::vecS, boost::vecS, 
    boost::bidirectionalS, boost::no_plugin, 
    boost::plugin<boost::edge_index, std::size_t> > Graph;

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

  int capacity[] = { 10, 20, 20, 20, 40, 40, 20, 20, 20, 10 };
  int flow[] = { 8, 12, 12, 12, 12, 12, 16, 16, 16, 8 };

  // insert edges into the graph, and assign each edge an ID number
  // to index into the property arrays
  boost::add_edge(G, 0, 1, 0);

  boost::add_edge(G, 1, 4, 1);
  boost::add_edge(G, 4, 7, 2);
  boost::add_edge(G, 7, 6, 3);

  boost::add_edge(G, 1, 3, 4);
  boost::add_edge(G, 3, 6, 5);

  boost::add_edge(G, 6, 5, 6);
  boost::add_edge(G, 5, 2, 7);
  boost::add_edge(G, 2, 1, 8);

  boost::add_edge(G, 6, 8, 9);

  typedef boost::property_map<Graph, boost::edge_index>::type EdgeID_PA;
  EdgeID_PA edge_id = boost::get_map(boost::edge_index(), G);

  typedef boost::random_access_iterator_property_map
    <int*, int, int&, EdgeID_PA> RA_PA;

  print_network(G, RA_PA(capacity, edge_id), RA_PA(flow, edge_id));
          
  return 0;
}
