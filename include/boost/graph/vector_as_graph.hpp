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

#ifndef BOOST_GRAPH_VEC_ADJ_LIST_HPP
#define BOOST_GRAPH_VEC_ADJ_LIST_HPP

#include <utility>
#include <vector>
#include <boost/iterator.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/pending/iterator_adaptors.hpp>

/*
  This module implements the VertexListGraph concept using a
  std::vector as the "back-bone" of the graph (the vector *is* the
  graph object). The edge-lists type of the graph is templated, so the
  user can choose any STL container, so long as the value_type of the
  container is convertible to the size_type of the vector. For now any
  graph properties must be stored seperately.

  This module requires the C++ compiler to support partial
  specialization for the graph_traits class, so this is not portable
  to VC++.

*/

namespace boost {
  namespace detail {
    template <class EdgeList> struct val_out_edge_ret;
    template <class EdgeList> struct val_out_edge_iter;
    template <class EdgeList> struct val_edge;
  }
}

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
namespace boost {
  template <class EdgeList>
  struct graph_traits< std::vector<EdgeList> >
  {
    typedef typename EdgeList::value_type V;
    typedef V vertex_descriptor;
    typedef typename detail::val_edge<EdgeList>::RET edge_descriptor;
    typedef typename EdgeList::const_iterator adjacency_iterator;
    typedef typename detail::val_out_edge_iter<EdgeList>::RET
      out_edge_iterator;
    typedef typename integer_range<V>::iterator vertex_iterator;
    typedef directed_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
  };
}
#endif

namespace boost {

  namespace detail {

    // "val" is short for Vector Adjacency List

    template <class EdgeList>
    struct val_edge
    {
      typedef typename EdgeList::value_type V;
      typedef std::pair<V,V> RET;
    };

    // need rewrite this using boost::iterator_adaptor
    template <class V, class Iter>
    class val_out_edge_iterator
      : public boost::iterator<std::input_iterator_tag, std::pair<V,V> >
    {
      typedef val_out_edge_iterator self;
      typedef std::pair<V,V> Edge;
    public:
      val_out_edge_iterator() { }
      val_out_edge_iterator(V s, Iter i) : _source(s), _iter(i) { }
      Edge operator*() { return Edge(_source, *_iter); }
      self& operator++() { ++_iter; return *this; }
      self operator++(int) { self t = *this; ++_iter; return t; }
      bool operator==(const self& x) const { return _iter == x._iter; }
      bool operator!=(const self& x) const { return _iter != x._iter; }
    protected:
      V _source;
      Iter _iter;
    };

    template <class EdgeList>
    struct val_out_edge_iter
    {
      typedef typename EdgeList::value_type V;
      typedef typename EdgeList::const_iterator Iter;
      typedef val_out_edge_iterator<V,Iter> RET;
    };

    template <class EdgeList>
    struct val_out_edge_ret
    {
      typedef typename val_out_edge_iter<EdgeList>::RET IncIter;
      typedef std::pair<IncIter,IncIter> RET;
    };

  } // namesapce detail

  template <class EdgeList, class Alloc>
  typename detail::val_out_edge_ret<EdgeList>::RET
  out_edges(typename EdgeList::value_type v, 
            const std::vector<EdgeList, Alloc>& g)
  {
    typedef typename detail::val_out_edge_iter<EdgeList>::RET Iter;
    typedef typename detail::val_out_edge_ret<EdgeList>::RET RET;
    return RET(Iter(v, g[v].begin()), Iter(v, g[v].end()));
  }

  template <class EdgeList, class Alloc>
  std::pair<typename EdgeList::const_iterator,
            typename EdgeList::const_iterator>
  adjacent_vertices(typename EdgeList::value_type v, 
                    const std::vector<EdgeList, Alloc>& g)
  {
    return std::make_pair(g[v].begin(), g[v].end());
  }

  // deprecated
  template <class EdgeList, class Alloc>
  std::pair<typename EdgeList::const_iterator,
            typename EdgeList::const_iterator>
  adj(typename EdgeList::value_type v, 
      const std::vector<EdgeList, Alloc>& g)
  {
    return adjacent_vertices(v, g);
  }

  // source() and target() already provided for pairs in graph_traits.hpp

  template <class EdgeList, class Alloc>
  std::pair<typename boost::integer_range<typename EdgeList::value_type>
              ::iterator,
            typename boost::integer_range<typename EdgeList::value_type>
              ::iterator >
  vertices(const std::vector<EdgeList, Alloc>& v)
  {
    typedef typename boost::integer_range<typename EdgeList::value_type>
      ::iterator Iter;
    return std::make_pair(Iter(0), Iter(v.size()));
  }

} // namespace boost

#endif // VEC_ADJ_LIST_HPP
