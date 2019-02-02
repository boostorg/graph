//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_MATRIX2GRAPH_HPP
#define BOOST_GRAPH_MATRIX2GRAPH_HPP

#include <utility>
#include <cstddef>
#include <iterator>
#include <boost/config.hpp>
#include <boost/operators.hpp>
#include <boost/pending/detail/int_iterator.hpp>
#include <boost/graph/graph_traits.hpp>

namespace boost {

  template <class Iter, class Vertex>
  class matrix_adj_iterator;

  template <class Iter, class Vertex>
  class matrix_incidence_iterator;

}

#define BOOST_GRAPH_ADAPT_MATRIX_TO_GRAPH(Matrix)  \
namespace boost { \
  template <> \
  struct graph_traits< Matrix > { \
    typedef Matrix::OneD::const_iterator Iter; \
    typedef Matrix::size_type V; \
    typedef V vertex_descriptor; \
    typedef Iter E; \
    typedef E edge_descriptor; \
    typedef boost::matrix_incidence_iterator<Iter, V> out_edge_iterator; \
    typedef boost::matrix_adj_iterator<Iter, V> adjacency_iterator; \
    typedef Matrix::size_type size_type; \
    typedef boost::int_iterator<size_type> vertex_iterator; \
    \
    friend graph_detail::iterator_range<vertex_iterator> \
    vertices(const Matrix& g) { \
      typedef vertex_iterator VIter; \
      return graph_detail::make_iterator_range(VIter(0), VIter(g.nrows())); \
    } \
    \
    friend graph_detail::iterator_range<out_edge_iterator> \
    out_edges(V v, const Matrix& g) { \
      typedef out_edge_iterator IncIter; \
      return graph_detail::make_iterator_range(IncIter(g[v].begin()), \
                            IncIter(g[v].end())); \
    } \
    friend graph_detail::iterator_range<adjacency_iterator> \
    adjacent_vertices(V v, const Matrix& g) { \
      typedef adjacency_iterator AdjIter; \
      return graph_detail::make_iterator_range(AdjIter(g[v].begin()), \
                            AdjIter(g[v].end())); \
    } \
    friend vertex_descriptor \
    source(E e, const Matrix& g) {  \
      return e.row();  \
    } \
    friend vertex_descriptor \
    target(E e, const Matrix& g) { \
      return e.column();  \
    } \
    friend size_type \
    num_vertices(const Matrix& g) { \
      return g.nrows();  \
    } \
    friend size_type \
    num_edges(const Matrix& g) { \
      return g.nnz(); \
    } \
    friend size_type \
    out_degree(V i, const Matrix& g) { \
      return g[i].nnz(); \
    } \
  }; \
}

namespace boost {

  template <class Iter, class Vertex>
  class matrix_adj_iterator
  {
    typedef matrix_adj_iterator self;
  public:
    typedef std::input_iterator_tag iterator_category;
    typedef Vertex value_type;
    typedef std::ptrdiff_t difference_type;
    typedef Vertex* pointer;
    typedef Vertex& reference;
    matrix_adj_iterator() { }
    matrix_adj_iterator(Iter i) : _iter(i) { }
    matrix_adj_iterator(const self& x) : _iter(x._iter) { }
    self& operator=(const self& x) { _iter = x._iter; return *this; }
    Vertex operator*() { return _iter.column(); }
    self& operator++() { ++_iter; return *this; }
    self operator++(int) { self t = *this; ++_iter; return t; }
    bool operator==(const self& x) const { return _iter == x._iter; }
    bool operator!=(const self& x) const { return _iter != x._iter; }
  protected:
    Iter _iter;
  };

  template <class Iter, class Vertex>
  class matrix_incidence_iterator
  {
    typedef matrix_incidence_iterator self;
  public:
    typedef std::input_iterator_tag iterator_category;
    typedef Iter value_type;
    typedef std::ptrdiff_t difference_type;
    typedef Iter* pointer;
    typedef Iter& reference;
    matrix_incidence_iterator() { }
    matrix_incidence_iterator(Iter i) : _iter(i) { }
    matrix_incidence_iterator(const self& x) : _iter(x._iter) { }
    self& operator=(const self& x) { _iter = x._iter; return *this; }
    Iter operator*() { return _iter; }
    self& operator++() { ++_iter; return *this; }
    self operator++(int) { self t = *this; ++_iter; return t; }
    bool operator==(const self& x) const { return _iter == x._iter; }
    bool operator!=(const self& x) const { return _iter != x._iter; }
  protected:
    Iter _iter;
  };
  
} /* namespace boost */

#endif /* BOOST_GRAPH_MATRIX2GRAPH_HPP*/
