//
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
#ifndef BOOST_GRAPH_TRANSPOSE_HPP
#define BOOST_GRAPH_TRANSPOSE_HPP

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>


namespace boost {

  // Transpose Graph
  //
  // To get the transpose of a directed graph. The transpose of a
  // directed graph <tt>G = (V, E)</tt> is the graph <tt>GT = (V,
  // ET)</tt>, where <tt>ET = {(v, u) in V x V: (u, v) in
  // E}</tt>. i.e., <tt>GT</tt> is <tt>G</tt> with all its edges
  // reversed.

  // Graph G2 must have the same number of vertices as G1,
  // and no edges. The edges will be added by calling
  // <tt>G2.add_edge(u,v)</tt>.

  // This version of <tt>transpose</tt> assumes the vertices
  // in the graph model VertexWithID. The other version
  // of transpose does not, are requests an ID Decorator
  // function from the user.
  //
  template <class Graph1, class Graph2> 
  void transpose_graph(const Graph1& G1, Graph2& G2)
  {
    transpose_graph(G1, G2, get_vertex_property(G1, id_tag()));
  }
  

  // Transpose  (with ID decorator function)
  //
  // To get the transpose of a directed graph. The transpose of a
  // directed graph <tt>G = (V, E)</tt> is the graph <tt>GT = (V,
  // ET)</tt>, where <tt>ET = {(v, u) in V x V: (u, v) in
  // E}</tt>. i.e., <tt>GT</tt> is <tt>G</tt> with all its edges
  // reversed.
  // <p>
  // Graph G2 must have the same number of vertices as G1,
  // and no edges. The edges will be added by calling
  // <tt>add_edge(G2, u,v)</tt>.
  // <p>
  // This version of <tt>transpose</tt> does not assume the vertices
  // in the graph model VertexWithID, but instead requests an ID
  // Decorator function from the user.  The other version of transpose
  // assumes VertexWithID.
  template <class Graph1, class Graph2, class ID> 
  void transpose_graph(const Graph1& G1, Graph2& G2, ID id )
  {
    typedef typename graph_traits<Graph1>::vertex_iterator VerIter;
    typedef typename graph_traits<Graph1>::
      out_edge_iterator OutEdgeIter;
    typedef typename graph_traits<Graph1>::vertex_descriptor Vertex;
    typedef typename graph_traits<Graph1>::edge_descriptor Edge;
    
    VerIter i, iend;
    OutEdgeIter j, jend;
    for ( boost::tie(i, iend) = vertices(G1); i!=iend; ++i) {
      Vertex u = *i;
      for (boost::tie(j,jend) = out_edges(u, G1); j !=jend; ++j) {
	Edge e = *j;
	Vertex v = target(e, G1);
	add_edge(G2, id[v], id[u]);
	// hmmm, what to do about edge plugins?  should have an
	// add-edge(g,e) function, and have a templated edge
	// constructor -JGS
      }
    }
  }


}

#endif /*BOOST_GRAPH_TRANSPOSE_H*/
