//=======================================================================
// Copyright 2001 Indiana University
// Author: Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_ITERATION_MACROS_HPP
#define BOOST_GRAPH_ITERATION_MACROS_HPP

#include <utility>

#define BGL_CAT(x,y) x ## y
#define BGL_RANGE(linenum) BGL_CAT(bgl_range_,linenum)
#define BGL_FIRST(linenum) (BGL_RANGE(linenum).first)
#define BGL_LAST(linenum) (BGL_RANGE(linenum).second)


#define BGL_FORALL_VERTICES_T(VNAME, GNAME, GraphType) \
for (std::pair<typename boost::graph_traits<GraphType>::vertex_iterator, \
               typename boost::graph_traits<GraphType>::vertex_iterator> BGL_RANGE(__LINE__) = vertices(GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
  for (typename boost::graph_traits<GraphType>::vertex_descriptor VNAME; \
    BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (VNAME = *BGL_FIRST(__LINE__), true):false; \
     ++BGL_FIRST(__LINE__))

#define BGL_FORALL_VERTICES(VNAME, GNAME, GraphType) \
for (std::pair<boost::graph_traits<GraphType>::vertex_iterator, \
               boost::graph_traits<GraphType>::vertex_iterator> BGL_RANGE(__LINE__) = vertices(GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
  for (boost::graph_traits<GraphType>::vertex_descriptor VNAME; \
    BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (VNAME = *BGL_FIRST(__LINE__), true):false; \
     ++BGL_FIRST(__LINE__))

#define BGL_FORALL_EDGES_T(ENAME, GNAME, GraphType) \
for (std::pair<typename boost::graph_traits<GraphType>::edge_iterator, \
               typename boost::graph_traits<GraphType>::edge_iterator> BGL_RANGE(__LINE__) = edges(GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
  for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME; \
    BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true):false; \
     ++BGL_FIRST(__LINE__))

#define BGL_FORALL_EDGES(ENAME, GNAME, GraphType) \
for (std::pair<boost::graph_traits<GraphType>::edge_iterator, \
               boost::graph_traits<GraphType>::edge_iterator> BGL_RANGE(__LINE__) = edges(GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
  for (boost::graph_traits<GraphType>::edge_descriptor ENAME; \
     BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true):false; \
     ++BGL_FIRST(__LINE__))

#define BGL_FORALL_ADJ_T(UNAME, VNAME, GNAME, GraphType) \
for (std::pair<typename boost::graph_traits<GraphType>::adjacency_iterator, \
               typename boost::graph_traits<GraphType>::adjacency_iterator> BGL_RANGE(__LINE__) = adjacent_vertices(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (typename boost::graph_traits<GraphType>::vertex_descriptor VNAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (VNAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_ADJ(UNAME, VNAME, GNAME, GraphType) \
for (std::pair<boost::graph_traits<GraphType>::adjacency_iterator, \
               boost::graph_traits<GraphType>::adjacency_iterator> BGL_RANGE(__LINE__) = adjacent_vertices(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (boost::graph_traits<GraphType>::vertex_descriptor VNAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (VNAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_OUTEDGES_T(UNAME, ENAME, GNAME, GraphType) \
for (std::pair<typename boost::graph_traits<GraphType>::out_edge_iterator, \
               typename boost::graph_traits<GraphType>::out_edge_iterator> BGL_RANGE(__LINE__) = out_edges(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_OUTEDGES(UNAME, ENAME, GNAME, GraphType) \
for (std::pair<boost::graph_traits<GraphType>::out_edge_iterator, \
               boost::graph_traits<GraphType>::out_edge_iterator> BGL_RANGE(__LINE__) = out_edges(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (boost::graph_traits<GraphType>::edge_descriptor ENAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_INEDGES_T(UNAME, ENAME, GNAME, GraphType) \
for (std::pair<typename boost::graph_traits<GraphType>::in_edge_iterator, \
               typename boost::graph_traits<GraphType>::in_edge_iterator> BGL_RANGE(__LINE__) = in_edges(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_INEDGES(UNAME, ENAME, GNAME, GraphType) \
for (std::pair<boost::graph_traits<GraphType>::in_edge_iterator, \
               boost::graph_traits<GraphType>::in_edge_iterator> BGL_RANGE(__LINE__) = in_edges(UNAME, GNAME); \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__); BGL_FIRST(__LINE__) = BGL_LAST(__LINE__)) \
for (boost::graph_traits<GraphType>::edge_descriptor ENAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? (ENAME = *BGL_FIRST(__LINE__), true) : false; \
   ++BGL_FIRST(__LINE__))

#endif // BOOST_GRAPH_ITERATION_MACROS_HPP
