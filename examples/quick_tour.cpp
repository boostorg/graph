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

#include <iostream>		         // for std::cout
#include <utility>		         // for std::pair
#include <algorithm>		         // for std::for_each
#include <boost/utility.hpp>             // for boost::tie
#include <boost/graph/graph_traits.hpp>  // for boost::graph_traits
#include <boost/graph/adjacency_list.hpp>

using namespace boost;

template <class Graph> struct exercise_vertex {
  exercise_vertex(Graph& g_) : g(g_) { }
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  void operator()(const Vertex& v) const
  {
    using namespace boost;
    typename vertex_property_accessor<Graph, id_tag>::type 
      vertex_id = get_vertex_property_accessor(g, id_tag());
    std::cout << "vertex: " << get(vertex_id, v) << std::endl;

    // Write out the outgoing edges
    std::cout << "\tout-edges: ";
    typename graph_traits<Graph>::out_edge_iterator out_i, out_end;
    typename graph_traits<Graph>::edge_descriptor e;
    for (tie(out_i, out_end) = out_edges(v, g); 
	 out_i != out_end; ++out_i)
    {
      e = *out_i;
      Vertex src = source(e, g), targ = target(e, g);
      std::cout << "(" << get(vertex_id, src)
		<< "," << get(vertex_id, targ) << ") ";
    }
    std::cout << std::endl;

    // Write out the incoming edges    
    std::cout << "\tin-edges: ";
    typename graph_traits<Graph>::in_edge_iterator in_i, in_end;
    for (tie(in_i, in_end) = in_edges(v, g); in_i != in_end; ++in_i)
    {
      e = *in_i;
      Vertex src = source(e, g), targ = target(e, g);
      std::cout << "(" << get(vertex_id, src)
                << "," << get(vertex_id, targ) << ") ";
    }
    std::cout << std::endl;

    // Write out all adjacent vertices    
    std::cout << "\tadjacent vertices: ";
    typename graph_traits<Graph>::adjacency_iterator ai, ai_end;
    for (tie(ai,ai_end) = adjacent_vertices(v, g);  ai != ai_end; ++ai)
      std::cout << get(vertex_id, *ai) <<  " ";
    std::cout << std::endl;
  }
  Graph& g;
};


int main(int,char*[])
{
  typedef std::pair<int,int> E;
  const int num_edges = 11;
  // writing out the edges in the graph
  E edge_array[num_edges] = { E(0,1), E(0,2), E(0,3), E(0,4), 
			      E(2,0), E(3,0), E(2,4), E(3,1), 
			      E(3,4), E(4,0), E(4,1) };
  const int num_vertices = 5;

  // create a typedef for the Graph type
  typedef adjacency_list<vecS, vecS, bidirectionalS> Graph;

  // declare a graph object
  Graph g(num_vertices);

  boost::vertex_property_accessor<Graph, id_tag>::type 
    vertex_id = get_vertex_property_accessor(g, id_tag());

  // add the edges to the graph object
  for (int i = 0; i < num_edges; ++i)
    add_edge(g, edge_array[i].first, edge_array[i].second);
  
  std::cout << "vertices(g) = ";
  typedef graph_traits<Graph>::vertex_iterator vertex_iter;
  std::pair<vertex_iter, vertex_iter> vp;
  for (vp = vertices(g); vp.first != vp.second; ++vp.first)
    std::cout << get(vertex_id, *vp.first) <<  " ";
  std::cout << std::endl;
  
  std::cout << "edges(g) = ";
  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
    std::cout << "(" << get(vertex_id, source(*ei, g)) 
	      << "," << get(vertex_id, target(*ei, g)) << ") ";
  std::cout << std::endl;
  
  std::for_each(vertices(g).first, vertices(g).second,
		exercise_vertex<Graph>(g));
  
  return 0;
}


