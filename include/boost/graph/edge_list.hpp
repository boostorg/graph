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

#ifndef BOOST_GRAPH_EDGE_LIST_HPP
#define BOOST_GRAPH_EDGE_LIST_HPP

#include <iterator>
#include <boost/config.hpp>
#include <boost/pending/ct_if.hpp>
#include <boost/pending/integer_range.hpp>
#include <boost/graph/graph_traits.hpp>

#ifndef __GNUC__
namespace boost {
#endif

//
// The edge_list class is an EdgeListGraph module that is constructed
// from a pair of iterators whose value type is a pair of vertex
// descriptors.
//
// For example:
//
//  typedef std::pair<int,int> E;
//  list<E> elist;
//  ...
//  typedef edge_list<list<E>::iterator> Graph;
//  Graph g(elist.begin(), elist.end());
//
// If the iterators are random access, then Graph::edge_descriptor
// is of Integral type, otherwise it is a struct, though it is
// convertible to an Integral type.
// 

// The implementation class for edge_list.
template <class G, class EdgeIter, class T, class D>
class edge_list_impl
{
public:
  typedef D Eid;
  typedef T Vpair;
  typedef typename Vpair::first_type V;
  typedef V vertex_descriptor;
    
  struct edge_descriptor
  {
    edge_descriptor() { }
    edge_descriptor(EdgeIter p, Eid id) : _ptr(p), _id(id) { }
    operator Eid() { return _id; }
    EdgeIter _ptr;
    Eid _id;
  };
  typedef edge_descriptor E;

  struct edge_iterator
  {
    typedef edge_iterator self;
    typedef E value_type;
    typedef E& reference;
    typedef E* pointer;
    typedef std::ptrdiff_t difference_type;
    typedef std::input_iterator_tag iterator_category;
    edge_iterator() { }
    edge_iterator(EdgeIter iter) : _iter(iter), _i(0) { }
    E operator*() { return E(_iter, _i); }
    self& operator++() { ++_iter; ++_i; return *this; }
    self operator++(int) { self t = *this; ++(*this); return t; }
    bool operator==(const self& x) { return _iter == x._iter; }
    bool operator!=(const self& x) { return _iter != x._iter; }
    EdgeIter _iter;
    Eid _i;
  };
  typedef void out_edge_iterator;
  typedef void in_edge_iterator;
  typedef void adjacency_iterator;
  typedef void vertex_iterator;

  typedef std::pair<edge_iterator,edge_iterator> EdgesRet;

  friend EdgesRet edges(const G& g) {
    return EdgesRet(edge_iterator(g._first), edge_iterator(g._last));
  }
  friend V source(E e, const G&) {
    return (*e._ptr).first;
  }
  friend V target(E e, const G&) {
    return (*e._ptr).second;
  }
};


// A specialized implementation for when the iterators are random access.

template <class G, class EdgeIter, class T, class D>
class edge_list_impl_ra
{
public:
  typedef D E;
  typedef T Vpair;
  typedef typename Vpair::first_type V;
  
  typedef E edge_descriptor;
  typedef V vertex_descriptor;
  typedef typename boost::integer_range<E>::iterator edge_iterator;
  typedef void out_edge_iterator;
  typedef void in_edge_iterator;
  typedef void adjacency_iterator;
  typedef void vertex_iterator;
  
  typedef std::pair<edge_iterator,edge_iterator> EdgesRet;

  friend EdgesRet edges(const G& g) {
    return EdgesRet(edge_iterator(0), edge_iterator(g._last - g._first));
  }    
  friend V source(E e, const G& g) {
    return g._first[e].first;
  }
  friend V target(E e, const G& g) {
    return g._first[e].second;
  }
};

#ifdef __GNUC__
namespace boost {
#endif

  // Some helper classes for determining if the iterators are random access
  template <class Cat>
  struct is_random { enum { RET = false }; };
  template <>
  struct is_random<std::random_access_iterator_tag> { enum { RET = true }; };

  // The edge_list class conditionally inherits from one of the
  // above two classes.

#if !defined BOOST_NO_STD_ITERATOR_TRAITS
  template <class EdgeIter, 
            class T = typename std::iterator_traits<EdgeIter>::value_type,
            class D = typename std::iterator_traits<EdgeIter>::difference_type >
  class edge_list
    : public ct_if< is_random<typename std::iterator_traits<EdgeIter>::iterator_category>::RET,
                    edge_list_impl_ra< edge_list<EdgeIter,T,D>, EdgeIter, T, D >,
                    edge_list_impl< edge_list<EdgeIter,T,D>, EdgeIter, T, D > 
             >::type
#else
  template <class EdgeIter, 
            class T,
            class D>
  class edge_list
    : public edge_list_impl< edge_list<EdgeIter,T,D>, EdgeIter, T, D >
#endif
  {
  public:
    typedef directed_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
    typedef std::size_t edges_size_type;
    typedef std::size_t vertices_size_type;
    typedef std::size_t degree_size_type;
    edge_list(EdgeIter first, EdgeIter last) : _first(first), _last(last) { 
      m_num_edges = std::distance(first, last);
    }
    edge_list(EdgeIter first, EdgeIter last, edges_size_type E)
      : _first(first), _last(last), m_num_edges(E) { }  
    
    EdgeIter _first, _last;
    edges_size_type m_num_edges;
  };

  template <class EdgeIter, class T, class D>
  std::size_t num_edges(const edge_list<EdgeIter, T, D>& el) {
    return el.m_num_edges;
  }

#ifndef BOOST_NO_STD_ITERATOR_TRAITS
  template <class EdgeIter>
  inline edge_list<EdgeIter>
  make_edge_list(EdgeIter first, EdgeIter last)
  {
    return edge_list<EdgeIter>(first, last);
  }
#endif
  
} /* namespace boost */

#endif /* BOOST_GRAPH_EDGE_LIST_HPP */
