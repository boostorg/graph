#ifndef BOOST_GRAPH_ITERATION_MACROS_HPP
#define BOOST_GRAPH_ITERATION_MACROS_HPP

#define BGL_CAT(x,y) x ## y
#define BGL_FIRST(linenum) BGL_CAT(bgl_first_,linenum)
#define BGL_LAST(linenum) BGL_CAT(bgl_last_,linenum)

/*
  BGL_FORALL_VERTICES(v, g, graph_t)  // This is on line 9
  exapands to the following, but all on the same line

  typename boost::graph_traits<graph_t>::vertex_iterator 
     bgl_first_9, bgl_last_9;
  tie(bgl_first_9, bgl_last_9) = vertices(g);
  for (typename boost::graph_traits<graph_t>::vertex_descriptor v;
       bgl_first_9 != bgl_last_9 ? (v = *bgl_first_9, true) : false;
       ++bgl_first_9)

  The other macros work in a similar fashion.
 */


#define BGL_FORALL_VERTICES(VNAME, GNAME, GraphType) \
typename boost::graph_traits<GraphType>::vertex_iterator BGL_FIRST(__LINE__),\
  BGL_LAST(__LINE__); \
tie(BGL_FIRST(__LINE__), BGL_LAST(__LINE__)) = vertices(GNAME); \
for (typename boost::graph_traits<GraphType>::vertex_descriptor VNAME; \
  BGL_FIRST(__LINE__) != BGL_LAST(__LINE__) ? \
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
