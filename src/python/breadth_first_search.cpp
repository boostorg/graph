#include <boost/graph/breadth_first_search.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "queue.hpp"

namespace boost { namespace graph { namespace python {

template<typename Graph>
class bfs_visitor
{
 public:
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename graph_traits<Graph>::edge_descriptor   edge_descriptor;

  virtual ~bfs_visitor() {}

  virtual void initialize_vertex(vertex_descriptor s, const Graph& g) const {}
  virtual void discover_vertex(vertex_descriptor u, const Graph& g) const {}
  virtual void examine_vertex(vertex_descriptor s, const Graph& g) const {}
  virtual void examine_edge(edge_descriptor e, const Graph& g) const {}
  virtual void tree_edge(edge_descriptor e, const Graph& g) const {}
  virtual void non_tree_edge(edge_descriptor e, const Graph& g) const {}
  virtual void gray_target(edge_descriptor e, const Graph& g) const {}
  virtual void black_target(edge_descriptor e, const Graph& g) const {}
  virtual void finish_vertex(vertex_descriptor s, const Graph& g) const {}
};

template<typename Graph>
class bfs_visitor_wrap 
  : public bfs_visitor<Graph>,
    public boost::python::wrapper<bfs_visitor<Graph> >
{
 public:
  typedef typename bfs_visitor<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename bfs_visitor<Graph>::edge_descriptor   edge_descriptor;

#define BGL_PYTHON_EVENT(Name,Descriptor)                       \
  void Name(Descriptor x, const Graph& g) const                 \
  {                                                             \
    if (override f = this->get_override(#Name))                 \
      f(x, boost::cref(g));                                     \
    else bfs_visitor<Graph>::Name(x, g);                        \
  }                                                             \
                                                                \
  void default_##Name(Descriptor x, const Graph& g)  const      \
  { this->bfs_visitor<Graph>::Name(x, g); }

  BGL_PYTHON_EVENT(initialize_vertex, vertex_descriptor)
  BGL_PYTHON_EVENT(examine_vertex, vertex_descriptor)
  BGL_PYTHON_EVENT(examine_edge, edge_descriptor)
  BGL_PYTHON_EVENT(tree_edge, edge_descriptor)
  BGL_PYTHON_EVENT(non_tree_edge, edge_descriptor)
  BGL_PYTHON_EVENT(gray_target, edge_descriptor)
  BGL_PYTHON_EVENT(black_target, edge_descriptor)
  BGL_PYTHON_EVENT(finish_vertex, vertex_descriptor)

#undef BGL_PYTHON_EVENT
};

template<typename Graph>
struct wrap_bfs_visitor_ref
{
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename graph_traits<Graph>::edge_descriptor   edge_descriptor;

  wrap_bfs_visitor_ref(const bfs_visitor<Graph>& v) : v(v) { }

  void initialize_vertex(vertex_descriptor s, const Graph& g) const
  { v.initialize_vertex(s, g); }

  void discover_vertex(vertex_descriptor u, const Graph& g) const
  { v.discover_vertex(u, g); }

  void examine_vertex(vertex_descriptor u, const Graph& g) const
  { v.examine_vertex(u, g); }

  void examine_edge(edge_descriptor e, const Graph& g)  const
  { v.examine_edge(e, g); }

  void tree_edge(edge_descriptor e, const Graph& g) const
  { v.tree_edge(e, g); }

  void non_tree_edge(edge_descriptor e, const Graph& g) const
  { v.non_tree_edge(e, g); }

  void gray_target(edge_descriptor e, const Graph& g) const
  { v.gray_target(e, g); }

  void black_target(edge_descriptor e, const Graph& g) const
  { v.black_target(e, g); }

  void finish_vertex(vertex_descriptor u, const Graph& g) const
  { v.finish_vertex(u, g); }

 private:
  const bfs_visitor<Graph>& v;
};

template<typename Graph> 
  class default_bfs_visitor : public bfs_visitor<Graph> {};

template<typename Graph>
void 
breadth_first_search_v
  (const Graph& g,
   typename Graph::Vertex s,
   const bfs_visitor<Graph>& visitor)
{
  if (dynamic_cast<const default_bfs_visitor<Graph>*>(&visitor)) {
    // No visitor. Do default BFS
    boost::breadth_first_search(g, s, 
                                vertex_index_map(g.get_vertex_index_map()));
  } else {
    boost::breadth_first_search
      (g, s, 
       vertex_index_map(g.get_vertex_index_map()).
       visitor(wrap_bfs_visitor_ref<Graph>(visitor)));
  }
}
 
template<typename Graph>
void 
breadth_first_search_qv
  (const Graph& g,
   typename Graph::Vertex s,
   const python_queue<typename Graph::Vertex>& Q)
//   const bfs_visitor<Graph>& visitor)
{
    boost::breadth_first_search(g, s, 
                                vertex_index_map(g.get_vertex_index_map()));
}


void export_breadth_first_search() 
{ 
  using boost::python::arg;

  // Breadth-first search algorithm
  def("breadth_first_search", &breadth_first_search_v<Graph>,
      (arg("graph"), "root_vertex", 
       arg("visitor") = default_bfs_visitor<Graph>()));

  def("breadth_first_search2", &breadth_first_search_qv<Graph>);
//      (arg("graph"), "root_vertex", 
//       arg("buffer") = python_queue<Graph::Vertex>::default_queue()));
}

template<typename Graph>
void export_breadth_first_search_in_graph()
{
  using boost::python::arg;

  // BFS visitor
  class_<bfs_visitor_wrap<Graph>, boost::noncopyable>("BFSVisitor")
    .def("initialize_vertex", &bfs_visitor<Graph>::initialize_vertex,
         &bfs_visitor_wrap<Graph>::default_initialize_vertex)
    .def("examine_vertex", &bfs_visitor<Graph>::examine_vertex,
         &bfs_visitor_wrap<Graph>::default_examine_vertex)
    .def("examine_edge", &bfs_visitor<Graph>::examine_edge,
         &bfs_visitor_wrap<Graph>::default_examine_edge)
    .def("tree_edge", &bfs_visitor<Graph>::tree_edge,
         &bfs_visitor_wrap<Graph>::default_tree_edge)
    .def("non_tree_edge", &bfs_visitor<Graph>::non_tree_edge,
         &bfs_visitor_wrap<Graph>::default_non_tree_edge)
    .def("gray_target", &bfs_visitor<Graph>::gray_target,
         &bfs_visitor_wrap<Graph>::default_gray_target)
    .def("black_target", &bfs_visitor<Graph>::black_target,
         &bfs_visitor_wrap<Graph>::default_black_target)
    .def("finish_vertex", &bfs_visitor<Graph>::finish_vertex,
         &bfs_visitor_wrap<Graph>::default_finish_vertex)
    ;

  class_<default_bfs_visitor<Graph>, bases<bfs_visitor<Graph> > >
    ("DefaultBFSVisitor", no_init);

  python_queue<typename Graph::Vertex>::declare("VertexQueue", 
                                                "DefaultVertexQueue");
}

template void export_breadth_first_search_in_graph<Graph>();
template void export_breadth_first_search_in_graph<Digraph>();

} } } // end namespace boost::graph::python
