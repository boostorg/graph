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

  template <class VertexListGraph, class MutableGraph> 
  void transpose_graph(const VertexListGraph& G, MutableGraph& G_T)
  {
    transpose_graph(G, G_T, get(vertex_index, G));
  }
  
  template <class VertexListGraph, class MutableGraph, class VertexIndexMap> 
  void transpose_graph(const VertexListGraph& G, MutableGraph& G_T,
                       VertexIndexMap index)
  {
    typedef graph_traits<VertexListGraph> Traits;
    typedef typename Traits::vertex_iterator VerIter;
    typedef typename Traits::out_edge_iterator OutEdgeIter;
    typedef typename Traits::vertex_descriptor Vertex;
    typedef typename Traits::edge_descriptor Edge;
    
    VerIter i, iend;
    OutEdgeIter j, jend;
    for (tie(i, iend) = vertices(G); i != iend; ++i) {
      Vertex u = *i;
      for (tie(j, jend) = out_edges(u, G); j != jend; ++j) {
        Edge e = *j;
        Vertex v = target(e, G);
        add_edge(vertex(get(index, v), G_T), vertex(get(index, u), G_T), G_T);
        // Hmmm, what to do about edge properties? Need to copy them
	// to the new graph.
      }
    }
  }


}

#endif /*BOOST_GRAPH_TRANSPOSE_H*/
