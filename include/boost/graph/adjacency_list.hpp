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

#ifndef BOOST_GRAPH_ADJACENCY_LIST_HPP
#define BOOST_GRAPH_ADJACENCY_LIST_HPP

/*
  Implementation Variation Points

  directed / undirected
  property/no property
  persistent/invalidating iterators 
  vertex descriptor type
  
 */
#include <vector>
#include <list>
#include <set>
#include <map>

#include <boost/config.hpp>

#if !defined BOOST_NO_HASH
#include <hash_set>
#include <hash_map>
#endif

#if !defined BOOST_NO_SLIST
#include <slist>
#endif

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>
#include <boost/pending/ct_if.hpp>
#include <boost/graph/detail/edge.hpp>

namespace boost {

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#if !defined BOOST_NO_SLIST
  struct slistS {};  
#endif

  struct vecS  { };
  struct listS { };
  struct setS { };
  struct mapS  { };
#if !defined BOOST_NO_HASH
  struct hash_mapS { };
  struct hash_setS { };
#endif

  template <class Selector, class ValueType>
  struct container_gen { };

  template <class ValueType>
  struct container_gen<listS, ValueType> {
    typedef std::list<ValueType> type;
  };
#if !defined BOOST_NO_SLIST
  template <class ValueType>
  struct container_gen<slistS, ValueType> {
    typedef std::slist<ValueType> type;
  };
#endif
  template <class ValueType>
  struct container_gen<vecS, ValueType> {
    typedef std::vector<ValueType> type;
  };

  template <class ValueType>
  struct container_gen<mapS, ValueType> {
    typedef std::set<ValueType> type;
  };

  template <class ValueType>
  struct container_gen<setS, ValueType> {
    typedef std::set<ValueType> type;
  };

#if !defined BOOST_NO_HASH
  template <class ValueType>
  struct container_gen<hash_mapS, ValueType> {
    typedef std::hash_set<ValueType> type;
  };

  template <class ValueType>
  struct container_gen<hash_setS, ValueType> {
    typedef std::hash_set<ValueType> type;
  };
#endif

#else // !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#if !defined BOOST_NO_SLIST
  struct slistS {
    template <class T>
    struct gen { typedef std::slist<T> type; };
  };
#endif

  struct vecS  {
    template <class T>
    struct bind { typedef std::vector<T> type; };
  };

  struct listS { 
    template <class T>
    struct bind { typedef std::list<T> type; };
  };

  struct setS  { 
    template <class T>
    struct bind { typedef std::set<T, std::less<T> > type; };
  };

#if !defined BOOST_NO_HASH
  struct hash_setS { 
    template <class T>
    struct bind { typedef std::hash_set<T, std::less<T> > type; };
  };
#endif

  struct mapS  { 
    template <class T>
    struct bind { typedef std::set<T, std::less<T> > type; };
  };

#if !defined BOOST_NO_HASH
  struct hash_mapS { 
    template <class T>
    struct bind { typedef std::hash_set<T, std::less<T> > type; };
  };
#endif

  template <class Selector> struct container_selector {
    typedef vecS type;
  };

#define BOOST_CONTAINER_SELECTOR(NAME) \
  template <> struct container_selector<NAME>  { \
    typedef NAME type; \
  }

  BOOST_CONTAINER_SELECTOR(vecS);
  BOOST_CONTAINER_SELECTOR(listS);
  BOOST_CONTAINER_SELECTOR(mapS);
#if !defined BOOST_NO_HASH
  BOOST_CONTAINER_SELECTOR(hash_mapS);
#endif
#if !defined BOOST_NO_SLIST
  BOOST_CONTAINER_SELECTOR(slistS);
#endif

  template <class Selector, class ValueType>
  struct container_gen {
    typedef typename container_selector<Selector>::type Select;
    typedef typename Select:: template bind<ValueType>::type type;
  };

#endif // !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

  struct directedS { enum { is_directed = true, is_bidir = false }; };
  struct undirectedS { enum { is_directed = false, is_bidir = false }; };
  struct bidirectionalS { enum { is_directed = true, is_bidir = true }; };

  template <class StorageSelector>
  struct parallel_edge_traits { };

  template <>
  struct parallel_edge_traits<vecS> { 
    typedef allow_parallel_edge_tag type; };

  template <>
  struct parallel_edge_traits<listS> { 
    typedef allow_parallel_edge_tag type; };

#if !defined BOOST_NO_SLIST
  template <>
  struct parallel_edge_traits<slistS> { 
    typedef allow_parallel_edge_tag type; };
#endif

  template <>
  struct parallel_edge_traits<setS> { 
    typedef disallow_parallel_edge_tag type; };

#if !defined BOOST_NO_HASH
  template <>
  struct parallel_edge_traits<hash_setS> {
    typedef disallow_parallel_edge_tag type; 
  };
#endif

  // mapS is obsolete, replaced with setS
  template <>
  struct parallel_edge_traits<mapS> { 
    typedef disallow_parallel_edge_tag type; };

#if !defined BOOST_NO_HASH
  template <>
  struct parallel_edge_traits<hash_mapS> {
    typedef disallow_parallel_edge_tag type; 
  };
#endif

  namespace detail {
    template <class Directed> struct is_random_access { 
      enum { value = false}; 
    };
    template <>
    struct is_random_access<vecS> { 
      enum { value = true }; 
    };

  } // namespace detail


  template <class EdgeListS = vecS,
            class VertexListS = vecS,
            class DirectedS = directedS>
  struct adjacency_list_traits
  {
    enum { is_rand_access = detail::is_random_access<VertexListS>::value };

    typedef typename boost::ct_if<DirectedS::is_bidir,
      bidirectional_tag,
      typename boost::ct_if<DirectedS::is_directed,
        directed_tag, undirected_tag
      >::type
    >::type directed_category;

    typedef typename parallel_edge_traits<EdgeListS>::type
      edge_parallel_category;

    typedef void* vertex_ptr;
    typedef typename boost::ct_if<is_rand_access,
      std::size_t, vertex_ptr>::type vertex_descriptor;

    typedef detail::bidir_edge<directed_category, vertex_descriptor> edge_descriptor;
  };

} // namespace boost

#include <boost/graph/detail/adjacency_list.hpp>

namespace boost {

  template <class EdgeListS = vecS,   // a Sequence or an AssociativeContainer
            class VertexListS = vecS, // a Sequence or a RandomAccessContainer
            class DirectedS = directedS,
            class VertexProperty = no_property,
            class EdgeProperty = no_property,
            class GraphProperty = no_property >
  class adjacency_list
    : public detail::adjacency_list_generator<
      adjacency_list<EdgeListS,VertexListS,DirectedS,
                     VertexProperty,EdgeProperty,GraphProperty>,
      VertexListS, EdgeListS, DirectedS, 
      VertexProperty, EdgeProperty, GraphProperty>::type
  {
    typedef adjacency_list self;
    typedef typename detail::adjacency_list_generator<
      self, VertexListS, EdgeListS, DirectedS, 
      VertexProperty, EdgeProperty, GraphProperty
    >::type Base;
  public:
    typedef typename Base::stored_vertex stored_vertex;

    typedef typename Base::vertices_size_type vertices_size_type;
    typedef typename Base::edges_size_type edges_size_type;
    typedef typename Base::degree_size_type degree_size_type;

    typedef EdgeProperty edge_property_type;
    typedef VertexProperty vertex_property_type;

    inline adjacency_list(const GraphProperty& p = GraphProperty()) 
      : m_property(p) { }

    inline adjacency_list(vertices_size_type num_vertices, 
                          const GraphProperty& p = GraphProperty())
      : Base(num_vertices), m_property(p) { }

    template <class EdgeIterator>
    inline adjacency_list(vertices_size_type num_vertices,
                          EdgeIterator first, EdgeIterator last,
                          const GraphProperty& p = GraphProperty())
      : Base(num_vertices, first, last), m_property(p) { }

    template <class EdgeIterator, class EdgePropertyIterator>
    inline adjacency_list(vertices_size_type num_vertices,
                          EdgeIterator first, EdgeIterator last,
                          EdgePropertyIterator ep_iter,
                          const GraphProperty& p = GraphProperty())
      : Base(num_vertices, first, last, ep_iter), m_property(p) { }

    //  protected:  (would be protected if friends were more portable)
    GraphProperty m_property;
  };

  template <class EL, class VL, class DS, class VP,class EP,class GP,class Tag>
  inline
  typename property_value<GP, Tag>::type&
  get_property(adjacency_list<EL,VL,DS,VP,EP,GP>& g, Tag) {
    typedef typename property_value<GP, Tag>::type value_type;
    return get_property_value(g.m_property, value_type(), Tag());
  }

  template <class EL, class VL, class DS, class VP,class EP,class GP,class Tag>
  inline
  const typename property_value<GP, Tag>::type&
  get_property(const adjacency_list<EL,VL,DS,VP,EP,GP>& g, Tag) {
    typedef typename property_value<GP, Tag>::type value_type;
    return get_property_value(g.m_property, value_type(), Tag());
  }

  template <class Directed, class Vertex, class Graph>
  inline Vertex
  source(const detail::edge_base<Directed,Vertex>& e, Graph& )
  {
    return e.m_source;
  }
  template <class Directed, class Vertex, class Graph>
  inline Vertex
  target(const detail::edge_base<Directed,Vertex>& e, Graph& )
  {
    return e.m_target;
  }

} // namespace boost


#endif // BOOST_GRAPH_ADJACENCY_LIST_HPP
