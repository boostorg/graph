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
#ifndef BOOST_GRAPH_TOPOLOGICAL_SORT_HPP
#define BOOST_GRAPH_TOPOLOGICAL_SORT_HPP

#include <boost/config.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>

namespace boost { 


  // Topological sort visitor
  //
  // This visitor merely writes the linear ordering into an
  // OutputIterator. The OutputIterator could be something like an
  // ostream_iterator, or it could be a back/front_insert_iterator.
  // Note that if it is a back_insert_iterator, the recorded order is
  // the reverse topological order. On the other hand, if it is a
  // front_insert_iterator, the recorded order is the topological
  // order.
  //
  template <class OutputIterator, class DFSVisitor>
  struct topo_sort_visitor : public DFSVisitor
  {
    topo_sort_visitor(OutputIterator _iter, DFSVisitor v)
      : DFSVisitor(v), m_iter(_iter) { }

    template <class Vertex, class Graph> 
    void finish_vertex(Vertex& u, Graph&) { *m_iter++ = u; }
    OutputIterator m_iter;
  };


  // Topological Sort
  //
  // The topological sort algorithm creates a linear ordering
  // of the vertices such that if edge (u,v) appears in the graph,
  // then u comes before v in the ordering. The graph must
  // be a directed acyclic graph (DAG). The implementation
  // consists mainly of a call to depth-first search.
  //
  // There are three overloaded versions of this function.
  template < class Graph, class OutputIterator, class Color, class DFSVisitor>
  inline void 
  topological_sort(Graph& G, OutputIterator iter, Color color, 
		   DFSVisitor visit)
  {
    typedef topo_sort_visitor<OutputIterator, DFSVisitor> TopoVisitor;
    depth_first_search(G, TopoVisitor(iter, visit), color);
  }

  template <class Graph, class OutputIterator, class DFSVisitor>
  inline void 
  topological_sort(Graph& G, OutputIterator iter, DFSVisitor visit)
  {
    topological_sort(G, iter, get(vertex_color, G), visit);
  }

  template <class Graph, class OutputIterator>
  inline void 
  topological_sort(Graph& G, OutputIterator iter)
  {
    topological_sort(G, iter, dfs_visitor<>());
  }

  } /* namespace */

#endif /*BOOST_GRAPH_TOPOLOGICAL_SORT_H*/
