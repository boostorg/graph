#ifndef BOOST_GRAPH_ITERATION_MACROS_HPP
#define BOOST_GRAPH_ITERATION_MACROS_HPP

#define BGL_CAT(x,y) x ## y
#define BGL_FIRST(line) BGL_CAT(first,line)
#define BGL_LAST(line) BGL_CAT(last,line)

#define BGL_FORALL_VERTICES(VNAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::vertex_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = vertices(GNAME); \
for (typename boost::graph_traits<GraphType>::vertex_descriptor VNAME \
  = *BGL_FIRST(__LINE__); BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
  (VNAME = *BGL_FIRST(__LINE__), true) : false;\
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_EDGES(ENAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::edge_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = edge(GNAME); \
for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME \
  = *BGL_FIRST(__LINE__); BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
  (ENAME = *BGL_FIRST(__LINE__), true) : false;\
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_ADJACENT(UNAME, VNAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::adjacency_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = adjacent_vertices(UNAME, GNAME); \
for (typename boost::graph_traits<GraphType>::vertex_descriptor VNAME \
  = *BGL_FIRST(__LINE__); BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
  (VNAME = *BGL_FIRST(__LINE__), true) : false;\
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_OUTEDGES(UNAME, ENAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::out_edge_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = out_edges(UNAME, GNAME); \
for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME \
  = *BGL_FIRST(__LINE__); BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
  (ENAME = *BGL_FIRST(__LINE__), true) : false;\
   ++BGL_FIRST(__LINE__))

#define BGL_FORALL_INEDGES(UNAME, ENAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::in_edge_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = in_edges(UNAME, GNAME); \
for (typename boost::graph_traits<GraphType>::edge_descriptor ENAME \
  = *BGL_FIRST(__LINE__); BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
  (ENAME = *BGL_FIRST(__LINE__), true) : false;\
   ++BGL_FIRST(__LINE__))


#endif // BOOST_GRAPH_ITERATION_MACROS_HPP
