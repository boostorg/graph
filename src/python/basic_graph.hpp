#ifndef BOOST_GRAPH_BASIC_GRAPH_HPP
#define BOOST_GRAPH_BASIC_GRAPH_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/vector_property_map.hpp>
#include <boost/dynamic_property_map.hpp>
#include <boost/python.hpp>

namespace boost { namespace graph { namespace python {

using namespace boost::python;

template<typename T> bool type_already_registered()
{
  return objects::registered_class_object(python::type_id<T>()).get() != 0;
}

template<typename Iterator>
class simple_python_iterator
{
public:
  simple_python_iterator(std::pair<Iterator, Iterator> p)
    : first(p.first), last(p.second) { }

  typename std::iterator_traits<Iterator>::value_type next() 
  { 
    if (first == last) objects::stop_iteration_error();
    return *first++;
  }

  static void declare(const char* name)
  {
    if (!type_already_registered<simple_python_iterator>())
      class_<simple_python_iterator<Iterator> >(name, no_init)
        .def("__iter__", objects::identity_function())
        .def("next", &simple_python_iterator<Iterator>::next)
      ;
  }

private:
  Iterator first;
  Iterator last;
};

template<typename PropertyMap>
struct declare_readable_property_map
{
  typedef typename property_traits<PropertyMap>::key_type   key_type;
  typedef typename property_traits<PropertyMap>::value_type value_type;

  static value_type getitem(const PropertyMap& pm, const key_type& key)
  { return get(pm, key); }

  static void declare(const char* name)
  {
    if (!type_already_registered<PropertyMap>())
      class_<PropertyMap>(name, no_init)
        .def("__getitem__", &getitem)
      ;
  }
};

template<typename PropertyMap>
struct declare_property_map
{
  typedef typename property_traits<PropertyMap>::key_type   key_type;
  typedef typename property_traits<PropertyMap>::value_type value_type;

  static value_type getitem(const PropertyMap& pm, const key_type& key)
  { return get(pm, key); }

  static void 
  setitem(const PropertyMap& pm, const key_type& key,const value_type& value)
  { return put(pm, key, value); }

  static void declare(const char* name)
  {
    if (!type_already_registered<PropertyMap>())
      class_<PropertyMap>(name, no_init)
        .def("__getitem__", &getitem)
        .def("__setitem__", &setitem)
      ;
  }
};

template<typename T, typename DirectedS>
struct basic_descriptor
{
  basic_descriptor() {}
  basic_descriptor(T base) : base(base) { }

  operator T() const { return base; }

  struct create 
  {
    typedef basic_descriptor result_type;
    basic_descriptor operator()(T base) const { return base; }
  };

  T base;
};

template<typename T, typename DirectedS>
inline bool 
operator==(const basic_descriptor<T, DirectedS>& u,
           const basic_descriptor<T, DirectedS>& v)
{ return u.base == v.base; }

template<typename T, typename DirectedS>
inline bool 
operator!=(const basic_descriptor<T, DirectedS>& u,
           const basic_descriptor<T, DirectedS>& v)
{ return u.base != v.base; }

template<typename Key, typename IndexMap>
struct basic_index_map
{
  typedef Key                                                  key_type;
  typedef typename property_traits<IndexMap>::value_type value_type;
  typedef typename property_traits<IndexMap>::reference  reference;
  typedef typename property_traits<IndexMap>::category   category;

  basic_index_map() {}
  basic_index_map(const IndexMap& id = IndexMap())
    : id(id) { }

  IndexMap id;
};

template<typename Key, typename IndexMap>
inline typename basic_index_map<Key, IndexMap>::value_type
get(const basic_index_map<Key, IndexMap>& pm, 
    typename basic_index_map<Key, IndexMap>::key_type const& key)
{ return get(pm.id, key.base); }

enum graph_file_kind { gfk_adjlist, gfk_graphviz };

template<typename DirectedS>
class basic_graph
  : public adjacency_list<listS, listS, DirectedS,
                          property<vertex_index_t, std::size_t>,
                          property<edge_index_t, std::size_t> >
{
  typedef adjacency_list<listS, listS, DirectedS,
                         property<vertex_index_t, std::size_t>,
                         property<edge_index_t, std::size_t> > inherited;
  typedef graph_traits<inherited> traits;

  typedef typename traits::vertex_iterator           base_vertex_iterator;
  typedef typename traits::edge_iterator             base_edge_iterator;
  typedef typename traits::out_edge_iterator         base_out_edge_iterator;
  typedef typename traits::in_edge_iterator          base_in_edge_iterator;
  typedef typename traits::adjacency_iterator        base_adjacency_iterator;
  typedef typename property_map<inherited, vertex_index_t>::const_type
                                                     base_vertex_index_map;
  typedef typename property_map<inherited, edge_index_t>::const_type
                                                     base_edge_index_map;
  typedef typename traits::vertex_descriptor         base_vertex_descriptor;
  typedef typename traits::edge_descriptor           base_edge_descriptor;

 public:
  typedef basic_descriptor<base_vertex_descriptor, DirectedS>
                                                     Vertex;
  typedef Vertex                                     vertex_descriptor;
  typedef basic_descriptor<base_edge_descriptor, DirectedS>
                                                     Edge;
  typedef Edge                                       edge_descriptor;
  typedef basic_index_map<Vertex, base_vertex_index_map>
                                                     VertexIndexMap;
  typedef basic_index_map<Edge, base_edge_index_map> EdgeIndexMap;
  typedef std::size_t                                vertices_size_type;
  typedef std::size_t                                edges_size_type;
  typedef std::size_t                                degree_size_type;
  typedef typename traits::directed_category         directed_category;
  typedef typename traits::edge_parallel_category    edge_parallel_category;
  typedef typename traits::traversal_category        traversal_category; 
  typedef transform_iterator<typename Vertex::create, base_vertex_iterator>
                                                     vertex_iterator;
  typedef transform_iterator<typename Edge::create, base_edge_iterator>
                                                     edge_iterator;
  typedef transform_iterator<typename Edge::create, base_out_edge_iterator>
                                                     out_edge_iterator;
  typedef transform_iterator<typename Edge::create, base_in_edge_iterator>
                                                     in_edge_iterator;
  typedef transform_iterator<typename Vertex::create, base_adjacency_iterator>
                                                     adjacency_iterator;

  basic_graph();
  basic_graph(const std::string& filename, graph_file_kind kind); 
  
  Vertex add_vertex();
  void clear_vertex(Vertex vertex);
  void remove_vertex(Vertex vertex);
  std::size_t num_vertices() const;
  std::pair<vertex_iterator, vertex_iterator> vertices() const;
  vertex_iterator vertices_begin() const;
  vertex_iterator vertices_end() const;
  
  Edge add_edge(Vertex u, Vertex v);
  void remove_edge(Edge edge);
  std::size_t num_edges() const;
  std::pair<edge_iterator, edge_iterator> edges() const;
  edge_iterator edges_begin() const;
  edge_iterator edges_end() const;
  
  // Incidence Graph concept
  Vertex source(Edge e) const;
  Vertex target(Edge e) const;
  std::pair<out_edge_iterator, out_edge_iterator> out_edges(Vertex u) const;
  simple_python_iterator<out_edge_iterator> py_out_edges(Vertex u) const;
  std::size_t out_degree(Vertex u) const;
  
  // Bidirectional Graph concept
  std::pair<in_edge_iterator, in_edge_iterator> in_edges(Vertex u) const;
  simple_python_iterator<in_edge_iterator> py_in_edges(Vertex u) const;
  std::size_t in_degree(Vertex u) const;
  
  // Adjacency Graph concept
  std::pair<adjacency_iterator, adjacency_iterator> 
  adjacent_vertices(Vertex u) const;
  
  simple_python_iterator<adjacency_iterator> 
  py_adjacent_vertices(Vertex u) const;
  
  // Vertex property maps
  VertexIndexMap get_vertex_index_map() const 
  { return get(vertex_index, base()); }
  
  template<typename T> 
  vector_property_map<T, VertexIndexMap> 
  get_vertex_map(const std::string& name);
  
  bool has_vertex_map(const std::string& name) const;
  
  // Edge property maps
  EdgeIndexMap get_edge_index_map() const 
  { return get(edge_index, base()); }
  
  template<typename T> 
  vector_property_map<T, EdgeIndexMap> 
  get_edge_map(const std::string& name);
  
  bool has_edge_map(const std::string& name) const;
  
  // Graph I/O
  void read_graphviz(const std::string& filename, 
                     const std::string& node_id = std::string("node_id"));
  
  void write_graphviz(const std::string& filename,
                      const std::string& node_id = std::string("node_id"));
  
  void write_graphviz_def(const std::string& filename)
  { write_graphviz(filename); }
  
  
  inherited&       base()       { return *this; }
  const inherited& base() const { return *this; }
  
 private:
  std::vector<vertex_descriptor> index_to_vertex;
  std::vector<edge_descriptor>   index_to_edge;
  dynamic_properties dp;
};

// Vertex List Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::vertex_iterator, 
                 typename basic_graph<DirectedS>::vertex_iterator>
vertices(const basic_graph<DirectedS>& g)
{ return g.vertices(); }

template<typename DirectedS>
inline std::size_t num_vertices(const basic_graph<DirectedS>& g) 
{ return g.num_vertices(); }

// Edge List Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::edge_iterator, 
                 typename basic_graph<DirectedS>::edge_iterator>
edges(const basic_graph<DirectedS>& g)
{ return g.edges(); }

template<typename DirectedS>
inline std::size_t num_edges(const basic_graph<DirectedS>& g) 
{ return g.num_edges(); }

// Incidence Graph concept
template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
source(typename basic_graph<DirectedS>::edge_descriptor e, 
       const basic_graph<DirectedS>& g)
{ return g.source(e); }

template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
target(typename basic_graph<DirectedS>::edge_descriptor e, 
       const basic_graph<DirectedS>& g)
{ return g.target(e); }

template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::out_edge_iterator, 
                 typename basic_graph<DirectedS>::out_edge_iterator>
out_edges(typename basic_graph<DirectedS>::vertex_descriptor u, 
          const basic_graph<DirectedS>& g)
{ return g.out_edges(u); }

template<typename DirectedS>
inline std::size_t 
out_degree(typename basic_graph<DirectedS>::vertex_descriptor u, 
           const basic_graph<DirectedS>& g)
{ return g.out_degree(u); }

// Bidirectional Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::in_edge_iterator, 
                 typename basic_graph<DirectedS>::in_edge_iterator>
in_edges(typename basic_graph<DirectedS>::vertex_descriptor u, 
         const basic_graph<DirectedS>& g)
{ return g.in_edges(u); }

template<typename DirectedS>
inline std::size_t 
in_degree(typename basic_graph<DirectedS>::vertex_descriptor u, 
          const basic_graph<DirectedS>& g)
{ return g.in_degree(u); }

// Adjacency Graph concept
template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::adjacency_iterator, 
                 typename basic_graph<DirectedS>::adjacency_iterator>
adjacent_vertices(typename basic_graph<DirectedS>::vertex_descriptor u, 
                  const basic_graph<DirectedS>& g)
{ return g.adjacent_vertices(u); }

// Mutable basic_graph<DirectedS> concept
template<typename DirectedS>
inline typename basic_graph<DirectedS>::vertex_descriptor 
add_vertex(basic_graph<DirectedS>& g)
{ return g.add_vertex(); }

template<typename DirectedS>
inline std::pair<typename basic_graph<DirectedS>::edge_descriptor, bool>
add_edge(typename basic_graph<DirectedS>::vertex_descriptor u, 
         typename basic_graph<DirectedS>::vertex_descriptor v, 
         basic_graph<DirectedS>& g)
{ return std::make_pair(g.add_edge(u, v), true); }
 
template<typename DirectedS>
void export_basic_graph(const char* name);

} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_BASIC_GRAPH_HPP
