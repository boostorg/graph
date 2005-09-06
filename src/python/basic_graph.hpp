// Copyright 2004-5 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_BASIC_GRAPH_HPP
#define BOOST_GRAPH_BASIC_GRAPH_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/iterator/transform_iterator.hpp>
#include <boost/vector_property_map.hpp>
#include <boost/dynamic_property_map.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/graph/plod_generator.hpp>
#include <boost/graph/small_world_generator.hpp>
#include <boost/python.hpp>
#include "point2d.hpp"
#include "generators.hpp"

namespace boost { namespace graph { namespace python {

template<typename T> bool type_already_registered()
{
  using boost::python::objects::registered_class_object;
  using boost::python::type_id;

  return registered_class_object(type_id<T>()).get() != 0;
}

template<typename Iterator>
class simple_python_iterator
{
public:
  typedef typename std::iterator_traits<Iterator>::difference_type
  difference_type;

  simple_python_iterator(std::pair<Iterator, Iterator> p)
    : orig_first(p.first), first(p.first), last(p.second), n(-1) { }

  typename std::iterator_traits<Iterator>::value_type next() 
  { 
    using boost::python::objects::stop_iteration_error;

    if (first == last) stop_iteration_error();
    return *first++;
  }

  difference_type len()
  { 
    if (n == -1) n = std::distance(first, last);
    return n; 
  }

  static void declare(const char* name)
  {
    using boost::python::class_;
    using boost::python::no_init;
    using boost::python::objects::identity_function;
    if (!type_already_registered<simple_python_iterator>())
      class_<simple_python_iterator<Iterator> >(name, no_init)
        .def("__iter__", identity_function())
        .def("__len__", &simple_python_iterator<Iterator>::len)
        .def("next", &simple_python_iterator<Iterator>::next)
      ;
  }

private:
  Iterator orig_first;
  Iterator first;
  Iterator last;
  difference_type n;
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
    using boost::python::class_;
    using boost::python::no_init;

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
    using boost::python::class_;
    using boost::python::no_init;

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

struct stored_minstd_rand
{
  stored_minstd_rand(int seed = 1) : gen(seed) { }

  minstd_rand gen;
};

template<typename DirectedS>
class basic_graph
  : public stored_minstd_rand,
    public adjacency_list<listS, listS, DirectedS,
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

  typedef erdos_renyi_iterator<minstd_rand, basic_graph<DirectedS> >
    er_iterator;
  typedef plod_iterator<minstd_rand, basic_graph<DirectedS> >
    sf_iterator;
  typedef small_world_iterator<minstd_rand, basic_graph<DirectedS> >
    sw_iterator;
  
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
  basic_graph(boost::python::object, 
              const std::string& name_map = std::string());
  basic_graph(const std::string& filename, graph_file_kind kind); 
  basic_graph(erdos_renyi, int seed);
  basic_graph(power_law_out_degree, int seed);
  basic_graph(small_world, int seed);

  bool is_directed() const
  { return is_convertible<directed_category, directed_tag>::value; }

  Vertex add_vertex();
  void clear_vertex(Vertex vertex);
  void remove_vertex(Vertex vertex);
  std::size_t num_vertices() const;
  std::pair<vertex_iterator, vertex_iterator> vertices() const;
  simple_python_iterator<vertex_iterator> py_vertices() const;
  vertex_iterator vertices_begin() const;
  vertex_iterator vertices_end() const;
  
  Edge add_edge(Vertex u, Vertex v);
  void remove_edge(Edge edge);
  std::size_t num_edges() const;
  std::pair<edge_iterator, edge_iterator> edges() const;
  simple_python_iterator<edge_iterator> py_edges() const;
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
  void read_adjlist(const std::string& filename, 
                    const std::string& node_id = std::string("node_id"));
  
  void write_adjlist(const std::string& filename,
                     const std::string& node_id = std::string("node_id"));

  void write_adjlist_def(const std::string& filename)
  { write_adjlist(filename); }

  void read_graphviz(const std::string& filename, 
                     const std::string& node_id = std::string("node_id"));
  
  void write_graphviz(const std::string& filename,
                      const std::string& node_id = std::string("node_id"));
  
  void write_graphviz_def(const std::string& filename)
  { write_graphviz(filename); }
  
  // Simple functions for Visual C++ 7.1 :(
  vector_property_map<default_color_type, VertexIndexMap>
  get_vertex_color_map(const std::string& name)
  { return get_vertex_map<default_color_type>(name); }

  vector_property_map<double, VertexIndexMap>
  get_vertex_double_map(const std::string& name)
  { return get_vertex_map<double>(name); }
  
  vector_property_map<int, VertexIndexMap>
  get_vertex_int_map(const std::string& name)
  { return get_vertex_map<int>(name); }
  
  vector_property_map<std::string, VertexIndexMap>
  get_vertex_string_map(const std::string& name)
  { return get_vertex_map<std::string>(name); }
  
  vector_property_map<boost::python::object, VertexIndexMap>
  get_vertex_object_map(const std::string& name)
  { return get_vertex_map<boost::python::object>(name); }
  
  vector_property_map<point2d, VertexIndexMap>
  get_vertex_point2d_map(const std::string& name)
  { return get_vertex_map<point2d>(name); }
  
  vector_property_map<default_color_type, EdgeIndexMap>
  get_edge_color_map(const std::string& name)
  { return get_edge_map<default_color_type>(name); }

  vector_property_map<double, EdgeIndexMap>
  get_edge_double_map(const std::string& name)
  { return get_edge_map<double>(name); }
  
  vector_property_map<int, EdgeIndexMap>
  get_edge_int_map(const std::string& name)
  { return get_edge_map<int>(name); }
  
  vector_property_map<std::string, EdgeIndexMap>
  get_edge_string_map(const std::string& name)
  { return get_edge_map<std::string>(name); }
  
  vector_property_map<boost::python::object, EdgeIndexMap>
  get_edge_object_map(const std::string& name)
  { return get_edge_map<boost::python::object>(name); }
  
  inherited&       base()       { return *this; }
  const inherited& base() const { return *this; }
  
  dynamic_properties&       get_dynamic_properties()       { return dp; }
  const dynamic_properties& get_dynamic_properties() const { return dp; }

protected:
  void renumber_vertices();
  void renumber_edges();
  
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

template<typename DirectedS>
typename basic_graph<DirectedS>::VertexIndexMap
get(vertex_index_t, const basic_graph<DirectedS>& g)
{ return g.get_vertex_index_map(); }

template<typename DirectedS>
typename basic_graph<DirectedS>::EdgeIndexMap
get(edge_index_t, const basic_graph<DirectedS>& g)
{ return g.get_edge_index_map(); }

template<typename DirectedS>
struct graph_pickle_suite : boost::python::pickle_suite
{
  typedef basic_graph<DirectedS> Graph;
  typedef typename Graph::vertex_descriptor Vertex;
  typedef typename Graph::edge_descriptor Edge;

  static
  boost::python::tuple
  getstate(boost::python::object g_obj);
  
  static
  void
  setstate(boost::python::object g_obj, boost::python::tuple state);
};

class python_dynamic_property_map
{
 public:
  virtual ~python_dynamic_property_map() {}

  virtual void copy_value(const any& to, const any& from) = 0;
  virtual boost::python::object get_python(const any& key) = 0;
};

template<typename PropertyMap,
         typename ValueType = typename property_traits<PropertyMap>::value_type>
class python_dynamic_adaptor
  : public boost::detail::dynamic_property_map_adaptor<PropertyMap>, 
    public python_dynamic_property_map
{
  typedef boost::detail::dynamic_property_map_adaptor<PropertyMap> inherited;

public:
  typedef typename property_traits<PropertyMap>::key_type key_type;

  explicit python_dynamic_adaptor(const PropertyMap& property_map)
    : inherited(property_map) { }

  virtual void copy_value(const any& to, const any& from)
  { 
    boost::put(this->base(), any_cast<key_type>(to), 
               boost::get(this->base(), any_cast<key_type>(from)));
  }

  virtual boost::python::object get_python(const any& key)
  {
#if defined(__GNUC__) && (__GNUC__ == 2) && (__GNUC_MINOR__ == 95)
    return boost::get(this->base(), any_cast<key_type>(key));
#else
    using boost::get;

    return boost::python::object(get(this->base(), any_cast<key_type>(key)));
#endif    
  }
};

template<typename PropertyMap>
class python_dynamic_adaptor<PropertyMap, boost::python::object>
  : public boost::detail::dynamic_property_map_adaptor<PropertyMap>, 
    public python_dynamic_property_map
{
  typedef boost::detail::dynamic_property_map_adaptor<PropertyMap> inherited;

public:
  typedef typename property_traits<PropertyMap>::key_type key_type;

  explicit python_dynamic_adaptor(const PropertyMap& property_map)
    : inherited(property_map) { }

  virtual void copy_value(const any& to, const any& from)
  { 
    boost::put(this->base(), any_cast<key_type>(to), 
               boost::get(this->base(), any_cast<key_type>(from)));
  }

  virtual std::string get_string(const any& key)
  {
    using boost::python::extract;
    using boost::python::str;
    return std::string(
             extract<const char*>(str(boost::get(this->base(),
                                                 any_cast<key_type>(key)))));
  }

  virtual boost::python::object get_python(const any& key)
  {
#if defined(__GNUC__) && (__GNUC__ == 2) && (__GNUC_MINOR__ == 95)
    return boost::get(this->base(), any_cast<key_type>(key));
#else
    using boost::get;

    return get(this->base(), any_cast<key_type>(key));
#endif    
  }
};

} } } // end namespace boost::graph::python

#if 0
// Triggers bugs in GCC
namespace boost {
  template<typename DirectedS>
  struct property_map<graph::python::basic_graph<DirectedS>, vertex_index_t>
  {
    typedef typename graph::python::basic_graph<DirectedS>::VertexIndexMap
      type;
    typedef type const_type;
  };

  template<typename DirectedS>
  struct property_map<graph::python::basic_graph<DirectedS>, edge_index_t>
  {
    typedef typename graph::python::basic_graph<DirectedS>::EdgeIndexMap
      type;
    typedef type const_type;
  };
}
#endif

#endif // BOOST_GRAPH_BASIC_GRAPH_HPP
