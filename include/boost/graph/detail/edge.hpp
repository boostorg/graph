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
#ifndef BOOST_GRAPH_DETAIL_EDGE_H
#define BOOST_GRAPH_DETAIL_EDGE_H

#include <iosfwd>

namespace boost {


  namespace  detail {
    template <class Directed, class Vertex>
    struct edge_base {
      inline edge_base() {} 
      inline edge_base(Vertex s, Vertex d)
        : m_source(s), m_target(d) { }
      Vertex m_source;
      Vertex m_target;
    };

    template <class Edge>
    inline bool edge_equal(const Edge& a, const Edge& b, directed_tag) {
      return a.m_source == b.m_source && a.m_target == b.m_target;
    }
    template <class Edge>
    inline bool edge_equal(const Edge& a, const Edge& b, undirected_tag) {
      return a.m_source == b.m_source && a.m_target == b.m_target
        || a.m_source == b.m_target && a.m_target == b.m_source;
    }
    
    template <class Edge>
    inline bool edge_less(const Edge& a, const Edge& b, directed_tag) {
      return a.m_source < b.m_source && a.m_target < b.m_target;
    }
    // yes, this is a bit weird to define less than for an undirected
    // edge, but we need it to for putting the edges in std::set.
    // Perhaps should just make a specialized functor to pass to set...
    template <class Edge>
    inline bool edge_less(const Edge& a, const Edge& b, undirected_tag) {
      return !edge_equal(a, v) && edge_less(a,b,directed_tag());
    }
    
    template <class Directed, class Vertex>
    class bidir_edge  : public edge_base<Directed,Vertex> {
      typedef bidir_edge                              self;
      typedef edge_base<Directed,Vertex> Base;
    public: 
      typedef void                              property_type;
      
      inline bidir_edge() : m_eproperty(0) {} 
      
      inline bidir_edge(Vertex s, Vertex d, const property_type* eplug = 0)
        : Base(s,d), m_eproperty(const_cast<property_type*>(eplug)) { }
      
      property_type* get_property() { return m_eproperty; }
      const property_type* get_property() const { return m_eproperty; }
      
      //  protected:
      property_type* m_eproperty;
    };
    

  template <class D, class V>
  inline bool
  operator==(const detail::edge_base<D,V>& a, const detail::edge_base<D,V>& b)
  {
    return detail::edge_equal(a, b, D());
  }
  template <class D, class V>
  inline bool
  operator!=(const detail::edge_base<D,V>& a, const detail::edge_base<D,V>& b)
  {
    return ! detail::edge_equal(a, b, D());
  }
  template <class D, class V>
  inline bool
  operator<(const detail::edge_base<D,V>& a, const detail::edge_base<D,V>& b)
  {
    return detail::edge_less(a, b, D());
  }


  } //namespace detail
  
} // namespace boost


#endif /*BOOST_GRAPH_DETAIL_DETAIL::EDGE_H*/
