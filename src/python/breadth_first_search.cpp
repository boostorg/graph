#include <boost/graph/breadth_first_search.hpp>
#include "graph.hpp"
#include "digraph.hpp"
#include "queue.hpp"

namespace boost { namespace graph { namespace python {

#define BGL_PYTHON_VISITOR bfs_visitor
#define BGL_PYTHON_EVENTS_HEADER "bfs_events.hpp"
#include "visitor.hpp"
#undef BGL_PYTHON_EVENTS_HEADER
#undef BGL_PYTHON_VISITOR

template<typename Graph>
void 
breadth_first_search_qvs
  (const Graph& g,
   typename Graph::Vertex s,
   python_queue<typename Graph::Vertex>& Q,
   const bfs_visitor<Graph>& visitor,
   const vector_property_map<default_color_type, 
                             typename Graph::VertexIndexMap>* in_color)
{
  typedef vector_property_map<default_color_type, 
                              typename Graph::VertexIndexMap> ColorMap;

  ColorMap color = 
    in_color? *in_color : ColorMap(g.num_vertices(), g.get_vertex_index_map());

  typedef typename python_queue<typename Graph::Vertex>::default_queue
    default_queue_type;

  bool has_default_buffer = dynamic_cast<default_queue_type*>(&Q);
  bool has_default_visitor = 
    dynamic_cast<typename bfs_visitor<Graph>::default_arg const*>(&visitor);

  if (has_default_buffer) {
    if (has_default_visitor) {
      boost::breadth_first_search(g, s, color_map(color));
    } else {
      boost::breadth_first_search
        (g, s, 
         color_map(color).
         visitor(typename bfs_visitor<Graph>::ref(visitor)));
    }
  } else {
    if (has_default_visitor) {
      boost::breadth_first_search(g, s, 
                                  buffer(Q).
                                  color_map(color));
    } else {
      boost::breadth_first_search
        (g, s, Q, typename bfs_visitor<Graph>::ref(visitor), color);
    }
  }
}
 
void export_breadth_first_search() 
{ 
  using boost::python::arg;

  def("breadth_first_search", &breadth_first_search_qvs<Graph>,
      (arg("graph"), "root_vertex", 
       arg("buffer") = python_queue<Graph::Vertex>::default_queue(),
       arg("visitor") = bfs_visitor<Graph>::default_arg(),
       arg("color_map") = 
         (vector_property_map<default_color_type, Graph::VertexIndexMap>*)0));

  def("breadth_first_search", &breadth_first_search_qvs<Digraph>,
      (arg("graph"), "root_vertex", 
       arg("buffer") = python_queue<Digraph::Vertex>::default_queue(),
       arg("visitor") = bfs_visitor<Digraph>::default_arg(),
       arg("color_map") = 
        (vector_property_map<default_color_type, Digraph::VertexIndexMap>*)0));
}

template<typename Graph>
void export_breadth_first_search_in_graph()
{
  bfs_visitor<Graph>::declare("BFSVisitor", "DefaultBFSVisitor");
  python_queue<typename Graph::Vertex>::declare("VertexQueue", 
                                                "DefaultVertexQueue");
}

template void export_breadth_first_search_in_graph<Graph>();
template void export_breadth_first_search_in_graph<Digraph>();

} } } // end namespace boost::graph::python
