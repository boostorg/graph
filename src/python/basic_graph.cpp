// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "basic_graph.hpp"
#include <boost/graph/iteration_macros.hpp>

namespace boost { 

inline std::ostream& operator<<(std::ostream& out, default_color_type c)
{
  switch (c) {
  case white_color: return out << "white";
  case gray_color: return out << "gray";
  case green_color: return out << "green";
  case red_color: return out << "red";
  case black_color: return out << "black";
  }
  return out;
}

inline std::istream& operator>>(std::istream& in, default_color_type& c)
{
  std::string text;
  if (in >> text) {
    if (text == "white") c = white_color;
    else if (text == "gray") c = gray_color;
    else if (text == "green") c = green_color;
    else if (text == "red") c = red_color;
    else if (text == "black") c = black_color;
    else {
      in.setstate(std::ios_base::failbit);
      return in;
    }
  }
  return in;
}

namespace graph { namespace python {

template<typename DirectedS>
struct build_string_property_maps
{
  build_string_property_maps(basic_graph<DirectedS>* g) : g(g) { }

  std::auto_ptr<boost::dynamic_property_map>
  operator()(const std::string& name, const boost::any& key, 
             const boost::any& value)
  {
    typedef typename basic_graph<DirectedS>::VertexIndexMap VertexIndexMap;
    typedef typename basic_graph<DirectedS>::EdgeIndexMap   EdgeIndexMap;

    std::auto_ptr<boost::dynamic_property_map> result(0);

    if (key.type() == typeid(typename basic_graph<DirectedS>::Vertex)) {
      typedef vector_property_map<std::string, VertexIndexMap>
        property_map_type;
      typedef python_dynamic_adaptor<property_map_type> adaptor_type;
      result.reset
        (new adaptor_type(property_map_type(g->num_vertices(),
                                            g->get_vertex_index_map())));
    } else if (key.type() == typeid(typename basic_graph<DirectedS>::Edge)) {
      typedef vector_property_map<std::string, EdgeIndexMap> property_map_type;
      typedef python_dynamic_adaptor<property_map_type> adaptor_type;
      result.reset
        (new adaptor_type(property_map_type(g->num_edges(),
                                            g->get_edge_index_map())));
    } 
    return result;
  }

private:
  basic_graph<DirectedS>* g;
};

// ----------------------------------------------------------
// Constructors
// ----------------------------------------------------------
template<typename DirectedS>
basic_graph<DirectedS>::basic_graph() 
  : inherited(), dp(build_string_property_maps<DirectedS>(this))
{ }

template<typename DirectedS>
basic_graph<DirectedS>::basic_graph(boost::python::object l,
                                    const std::string& name_map)
  : inherited()
{
  using boost::python::object;
  std::map<object, vertex_descriptor> verts;
  int len = ::boost::python::extract<int>(l.attr("__len__")());

  vector_property_map<object, VertexIndexMap> name;
  if (!name_map.empty()) name = get_vertex_map<object>(name_map);

  for (int  i = 0; i < len; ++i) {
    vertex_descriptor u, v;
    object up = l[i][0];
    object vp = l[i][1];

    typename std::map<object, vertex_descriptor>::iterator pos;
    pos = verts.find(up);
    if (pos == verts.end()) {
      u = verts[up] = add_vertex();
      if (!name_map.empty()) name[u] = up;
    }
    else u = pos->second;
    pos = verts.find(vp);
    if (pos == verts.end()) {
      v = verts[vp] = add_vertex();
      if (!name_map.empty()) name[v] = vp;
    }
    else v = pos->second;

    add_edge(u, v);
  }
}

template<typename DirectedS>
basic_graph<DirectedS>::basic_graph(const std::string& filename, 
                                    graph_file_kind kind)
  : inherited(), dp(build_string_property_maps<DirectedS>(this))
{
  switch (kind) {
  case gfk_adjlist:
    break;
  case gfk_graphviz:
    read_graphviz(filename);
    break;
  }
}

template<typename DirectedS>
basic_graph<DirectedS>::basic_graph(erdos_renyi er, int seed)
  : stored_minstd_rand(seed), 
    inherited(er_iterator(this->gen, er.n, er.p), er_iterator(), er.n) 
{ 
  renumber_vertices();
  renumber_edges();
}

template<typename DirectedS>
basic_graph<DirectedS>::basic_graph(power_law_out_degree plod, int seed)
  : stored_minstd_rand(seed), 
    inherited(sf_iterator(this->gen, plod.n, plod.alpha, plod.beta), 
              sf_iterator(),
              plod.n)
{ 
  renumber_vertices();
  renumber_edges();
}

template<typename DirectedS>
basic_graph<DirectedS>::basic_graph(small_world sw, int seed)
  : stored_minstd_rand(seed), 
    inherited(sw_iterator(this->gen, sw.n, sw.k, sw.p), sw_iterator(), sw.n)
{ 
  renumber_vertices();
  renumber_edges();
}

// ----------------------------------------------------------
// Incidence basic_graph<DirectedS> concept
// ----------------------------------------------------------
template<typename DirectedS>
typename basic_graph<DirectedS>::Vertex 
basic_graph<DirectedS>::source(Edge e) const
{ return Vertex(boost::source(e.base, base())); }

template<typename DirectedS>
typename basic_graph<DirectedS>::Vertex 
basic_graph<DirectedS>::target(Edge e) const
{ return Vertex(boost::target(e.base, base())); }


template<typename DirectedS>
std::pair<typename basic_graph<DirectedS>::out_edge_iterator, 
          typename basic_graph<DirectedS>::out_edge_iterator> 
basic_graph<DirectedS>::out_edges(Vertex u) const
{
  std::pair<base_out_edge_iterator, base_out_edge_iterator> rng = 
    boost::out_edges(u.base, base());
  return std::make_pair(out_edge_iterator(rng.first, typename Edge::create()),
                        out_edge_iterator(rng.second,typename Edge::create()));
}

template<typename DirectedS>
simple_python_iterator<typename basic_graph<DirectedS>::out_edge_iterator>
basic_graph<DirectedS>::py_out_edges(Vertex u) const
{ 
  return simple_python_iterator<out_edge_iterator>(out_edges(u));
}

template<typename DirectedS>
std::size_t
basic_graph<DirectedS>::out_degree(Vertex u) const
{
  return boost::out_degree(u.base, base());
}

// ----------------------------------------------------------
// Bidirectional basic_graph<DirectedS> concept
// ----------------------------------------------------------
template<typename DirectedS>
std::pair<typename basic_graph<DirectedS>::in_edge_iterator, 
          typename basic_graph<DirectedS>::in_edge_iterator> 
basic_graph<DirectedS>::in_edges(Vertex u) const
{
  std::pair<base_in_edge_iterator, base_in_edge_iterator> rng = 
    boost::in_edges(u.base, base());
  return std::make_pair(in_edge_iterator(rng.first, typename Edge::create()),
                        in_edge_iterator(rng.second, typename Edge::create()));
}

template<typename DirectedS>
simple_python_iterator<typename basic_graph<DirectedS>::in_edge_iterator>
basic_graph<DirectedS>::py_in_edges(Vertex u) const
{ 
  return simple_python_iterator<in_edge_iterator>(in_edges(u));
}

template<typename DirectedS>
std::size_t
basic_graph<DirectedS>::in_degree(Vertex u) const
{
  return boost::in_degree(u.base, base());
}

// ----------------------------------------------------------
// Adjacency basic_graph<DirectedS> concept
// ----------------------------------------------------------
template<typename DirectedS>
std::pair<typename basic_graph<DirectedS>::adjacency_iterator, 
          typename basic_graph<DirectedS>::adjacency_iterator> 
basic_graph<DirectedS>::adjacent_vertices(Vertex u) const
{
  std::pair<base_adjacency_iterator, base_adjacency_iterator> rng = 
    boost::adjacent_vertices(u.base, base());
  return std::make_pair(adjacency_iterator(rng.first, typename Vertex::create()),
                        adjacency_iterator(rng.second, typename Vertex::create()));
}

template<typename DirectedS>
simple_python_iterator<typename basic_graph<DirectedS>::adjacency_iterator>
basic_graph<DirectedS>::py_adjacent_vertices(Vertex u) const
{ 
  return 
    simple_python_iterator<adjacency_iterator>(adjacent_vertices(u));
}

// ----------------------------------------------------------
// Vertex properties
// ----------------------------------------------------------
template<typename DirectedS>
template<typename T> 
vector_property_map<T, typename basic_graph<DirectedS>::VertexIndexMap> 
basic_graph<DirectedS>::get_vertex_map(const std::string& name)
{
  typedef vector_property_map<T, VertexIndexMap> result_type;
  typedef python_dynamic_adaptor<result_type> adaptor_type;
  
  dynamic_properties::iterator i = dp.lower_bound(name);
  while (i != dp.end() && i->first == name) {
    if (i->second->key() == typeid(Vertex)) {
      if (i->second->value() == typeid(T)) {
        return dynamic_cast<adaptor_type&>(*i->second).base();
      } else {
        // Convert the property map element-by-element to the
        // requested type.
        result_type result(num_vertices(), get_vertex_index_map());
        for(vertex_iterator v = vertices().first; v != vertices().second;
            ++v) {
          put(result, *v, lexical_cast<T>(i->second->get_string(*v)));
        }
        
        // Replace the existing property map with the converted one
        adaptor_type* adaptor = new adaptor_type(result);
        delete i->second;
        i->second = adaptor;
        
        // Return the new property map.
        return result;
      }
    }
    ++i;
  }
  
  typedef vector_property_map<T, VertexIndexMap> property_map_type;
  typedef python_dynamic_adaptor<property_map_type> adaptor_type;
  property_map_type result(num_vertices(), get_vertex_index_map());
  dp.insert(name, 
            std::auto_ptr<dynamic_property_map>(new adaptor_type(result)));
  return result;
}

template<typename DirectedS>
bool
basic_graph<DirectedS>::has_vertex_map(const std::string& name) const
{
  dynamic_properties::const_iterator i = dp.lower_bound(name);
  while (i != dp.end() && i->first == name) {
    if (i->second->key() == typeid(Vertex))
      return true;
  }
  return false;
}

// ----------------------------------------------------------
// Edge properties
// ----------------------------------------------------------
template<typename DirectedS>
template<typename T> 
vector_property_map<T, typename basic_graph<DirectedS>::EdgeIndexMap> 
basic_graph<DirectedS>::get_edge_map(const std::string& name)
{
  typedef vector_property_map<T, EdgeIndexMap> result_type;
  typedef python_dynamic_adaptor<result_type> adaptor_type;
  
  dynamic_properties::iterator i = dp.lower_bound(name);
  while (i != dp.end() && i->first == name) {
    if (i->second->key() == typeid(Edge)) {
      if (i->second->value() == typeid(T)) {
        return dynamic_cast<adaptor_type&>(*i->second).base();
      } else {
        // Convert the property map element-by-element to the
        // requested type.
        result_type result(num_vertices(), get_edge_index_map());
        for(edge_iterator e = edges().first; e != edges().second; ++e) {
          put(result, *e, lexical_cast<T>(i->second->get_string(*e)));
        }
        
        // Replace the existing property map with the converted one
        adaptor_type* adaptor = new adaptor_type(result);
        delete i->second;
        i->second = adaptor;
        
        // Return the new property map.
        return result;
      }
    }
    ++i;
  }

  typedef vector_property_map<T, EdgeIndexMap> property_map_type;
  typedef python_dynamic_adaptor<property_map_type> adaptor_type;
  property_map_type result(num_edges(), get_edge_index_map());
  dp.insert(name, 
            std::auto_ptr<dynamic_property_map>(new adaptor_type(result)));
  return result;
}

template<typename DirectedS>
bool
basic_graph<DirectedS>::has_edge_map(const std::string& name) const
{
  dynamic_properties::const_iterator i = dp.lower_bound(name);
  while (i != dp.end() && i->first == name) {
    if (i->second->key() == typeid(Edge))
      return true;
  }
  return false;
}

// ----------------------------------------------------------
// Mutable graph
// ----------------------------------------------------------
template<typename DirectedS>
typename basic_graph<DirectedS>::Vertex basic_graph<DirectedS>::add_vertex()
{
  using boost::add_vertex;

  base_vertex_descriptor v = add_vertex(base());
  put(vertex_index, base(), v, index_to_vertex.size());
  index_to_vertex.push_back(v);
  return Vertex(v);
}

template<typename DirectedS>
void basic_graph<DirectedS>::clear_vertex(Vertex vertex)
{
  // Remove all incoming and outgoing edges
  while (out_degree(vertex) > 0) remove_edge(*out_edges(vertex).first);
  while (in_degree(vertex) > 0) remove_edge(*in_edges(vertex).first);
}

template<typename DirectedS>
void basic_graph<DirectedS>::remove_vertex(Vertex vertex)
{
  using boost::remove_vertex;

  // Update property maps
  for (dynamic_properties::iterator i = dp.begin(); i != dp.end(); ++i) {
    if (i->second->key() == typeid(Vertex)) {
      dynamic_cast<python_dynamic_property_map*>(&*i->second)->
        copy_value(vertex, Vertex(index_to_vertex.back()));
    }
  }

  // Update vertex indices
  std::size_t index = get(vertex_index, base(), vertex.base);
  index_to_vertex[index] = index_to_vertex.back();
  put(vertex_index, base(), index_to_vertex[index], index);
  index_to_vertex.pop_back();

  // Remove the actual vertex
  remove_vertex(vertex.base, base());
}

template<typename DirectedS>
std::size_t basic_graph<DirectedS>::num_vertices() const
{
  using boost::num_vertices;
  return num_vertices(base());
}

template<typename DirectedS>
typename basic_graph<DirectedS>::vertex_iterator
basic_graph<DirectedS>::vertices_begin() const
{
  return make_transform_iterator(boost::vertices(base()).first, 
                                 typename Vertex::create());
}

template<typename DirectedS>
typename basic_graph<DirectedS>::vertex_iterator
basic_graph<DirectedS>::vertices_end() const
{
  return make_transform_iterator(boost::vertices(base()).second, 
                                 typename Vertex::create());
}

template<typename DirectedS>
std::pair<typename basic_graph<DirectedS>::vertex_iterator, 
          typename basic_graph<DirectedS>::vertex_iterator> 
basic_graph<DirectedS>::vertices() const
{
  return std::make_pair(vertices_begin(), vertices_end());
}

template<typename DirectedS>
typename basic_graph<DirectedS>::Edge 
basic_graph<DirectedS>::add_edge(Vertex u, Vertex v)
{
  using boost::add_edge;

  base_edge_descriptor e = add_edge(u.base, v.base, base()).first;
  put(edge_index, base(), e, index_to_edge.size());
  index_to_edge.push_back(e);
  return Edge(e);
}

template<typename DirectedS>
void basic_graph<DirectedS>::remove_edge(Edge edge)
{
  using boost::remove_edge;

  // Update property maps
  for (dynamic_properties::iterator i = dp.begin(); i != dp.end(); ++i) {
    if (i->second->key() == typeid(Edge)) {
      dynamic_cast<python_dynamic_property_map*>(&*i->second)->
        copy_value(edge, Edge(index_to_edge.back()));
    }
  }

  // Update edge indices
  std::size_t index = get(edge_index, base(), edge.base);
  index_to_edge[index] = index_to_edge.back();
  put(edge_index, base(), index_to_edge[index], index);
  index_to_edge.pop_back();

  // Remove the actual edge
  remove_edge(edge.base, base());
}

template<typename DirectedS>
std::size_t basic_graph<DirectedS>::num_edges() const
{
  using boost::num_edges;
  return num_edges(base());
}

template<typename DirectedS>
typename basic_graph<DirectedS>::edge_iterator
basic_graph<DirectedS>::edges_begin() const
{
  return make_transform_iterator(boost::edges(base()).first, 
                                 typename Edge::create());
}

template<typename DirectedS>
typename basic_graph<DirectedS>::edge_iterator
basic_graph<DirectedS>::edges_end() const
{
  return make_transform_iterator(boost::edges(base()).second, 
                                 typename Edge::create());
}

template<typename DirectedS>
std::pair<typename basic_graph<DirectedS>::edge_iterator, 
          typename basic_graph<DirectedS>::edge_iterator> 
basic_graph<DirectedS>::edges() const
{
  return std::make_pair(edges_begin(), edges_end());
}

template<typename DirectedS>
void basic_graph<DirectedS>::renumber_vertices()
{
  using boost::vertices;

  BGL_FORALL_VERTICES_T(v, base(), inherited) {
    put(vertex_index, base(), v, index_to_vertex.size());
    index_to_vertex.push_back(v);
  }
}

template<typename DirectedS>
void basic_graph<DirectedS>::renumber_edges()
{
  using boost::edges;

  BGL_FORALL_EDGES_T(e, base(), inherited) {
    put(edge_index, base(), e, index_to_edge.size());
    index_to_edge.push_back(e);
  }
}

template<typename Graph> void export_in_graph();

template<typename DirectedS>
void export_basic_graph(const char* name)
{
  using boost::python::arg;
  using boost::python::class_;
  using boost::python::init;
  using boost::python::object;
  using boost::python::range;
  using boost::python::scope;
  using boost::python::self;
  using boost::python::tuple;

  typedef basic_graph<DirectedS> Graph;
  typedef typename Graph::Vertex Vertex;
  typedef typename Graph::Edge Edge;
  typedef typename Graph::VertexIndexMap VertexIndexMap;
  typedef typename Graph::EdgeIndexMap EdgeIndexMap;

  {
    scope s(
      class_<basic_graph<DirectedS> >(name)
        // Constructors
        .def(init<object>())
        .def(init<object, std::string>())
        .def(init<std::string, graph_file_kind>())
        .def(init<erdos_renyi, std::size_t>(
               (arg("generator"), arg("seed") = 1)))
        .def(init<power_law_out_degree, std::size_t>(
               (arg("generator"), arg("seed") = 1)))
        .def(init<small_world, std::size_t>(
               (arg("generator"), arg("seed") = 1)))
        .def("is_directed", &Graph::is_directed)
        // Vertex List Graph concept
        .def("num_vertices", &Graph::num_vertices)
        .add_property("vertices", range(&Graph::vertices_begin, 
                                        &Graph::vertices_end))
        // Edge List Graph concept
        .def("num_edges", &Graph::num_edges)
        .add_property("edges", range(&Graph::edges_begin, &Graph::edges_end))
        // Mutable Graph concept
        .def("add_vertex", &Graph::add_vertex)
        .def("clear_vertex", &Graph::clear_vertex)
        .def("remove_vertex", &Graph::remove_vertex)
        .def("add_edge", &Graph::add_edge)
        .def("remove_edge", &Graph::remove_edge)
        // Incidence Graph concept
        .def("source", &Graph::source)
        .def("target", &Graph::target)
        .def("out_edges", &Graph::py_out_edges)
        .def("out_degree", &Graph::out_degree)
        .def("in_edges", &Graph::py_in_edges)
        .def("in_degree", &Graph::in_degree)
        .def("adjacent_vertices", &Graph::py_adjacent_vertices)
        // Vertex property maps
        .def("has_vertex_map", &Graph::has_vertex_map)
        .def("get_vertex_index_map", &Graph::get_vertex_index_map)
        .def("get_vertex_color_map", &Graph::get_vertex_color_map)
        .def("get_vertex_double_map", &Graph::get_vertex_double_map)
        .def("get_vertex_int_map", &Graph::get_vertex_int_map)
        .def("get_vertex_string_map", &Graph::get_vertex_string_map)
        .def("get_vertex_object_map", &Graph::get_vertex_object_map)
        .def("get_vertex_point2d_map", &Graph::get_vertex_point2d_map)
        // Edge property maps
        .def("has_edge_map", &Graph::has_edge_map)
        .def("get_edge_index_map", &Graph::get_edge_index_map)
        .def("get_edge_color_map", &Graph::get_edge_color_map)
        .def("get_edge_double_map", &Graph::get_edge_double_map)
        .def("get_edge_int_map", &Graph::get_edge_int_map)
        .def("get_edge_string_map", &Graph::get_edge_string_map)
        .def("get_edge_object_map", &Graph::get_edge_object_map)
        // Graph I/O
        .def("read_graphviz", &Graph::read_graphviz)
        .def("write_graphviz", &Graph::write_graphviz)
        .def("write_graphviz", &Graph::write_graphviz_def)
        // Pickling
        .def_pickle(graph_pickle_suite<DirectedS>())
      );
      
    export_in_graph<Graph>();
  }

  if (!type_already_registered<Vertex>())
    class_<Vertex>("Vertex")
      .def(self == self)
      .def(self != self);

  if (!type_already_registered<Edge>())
    class_<Edge>("Edge")
      .def(self == self)
      .def(self != self);

  // Iterators
  simple_python_iterator<typename Graph::out_edge_iterator>
    ::declare("out_edge_iterator");
  simple_python_iterator<typename Graph::in_edge_iterator>
    ::declare("in_edge_iterator");
  simple_python_iterator<typename Graph::adjacency_iterator>
    ::declare("adjacency_iterator");
  
  // Vertex property maps
  declare_readable_property_map<VertexIndexMap>
    ::declare("vertex_index_map");
  declare_property_map<vector_property_map<Vertex, VertexIndexMap> >
    ::declare("vertex_vertex_map");
  declare_property_map<vector_property_map<double, VertexIndexMap> >
    ::declare("vertex_double_map");
  declare_property_map<vector_property_map<int, VertexIndexMap> >
    ::declare("vertex_int_map");
  declare_property_map<vector_property_map<std::string, VertexIndexMap> >
    ::declare("vertex_string_map");
  declare_property_map<vector_property_map<object, VertexIndexMap> >
    ::declare("vertex_object_map");
  declare_property_map<vector_property_map<default_color_type, 
                                           VertexIndexMap> >
    ::declare("vertex_color_map");
  declare_property_map<vector_property_map<point2d, VertexIndexMap> >
    ::declare("vertex_point2d_map");

  // Edge property maps
  declare_readable_property_map<EdgeIndexMap>
    ::declare("edge_index_map");
  declare_property_map<vector_property_map<Edge, EdgeIndexMap> >
    ::declare("edge_edge_map");
  declare_property_map<vector_property_map<double, EdgeIndexMap> >
    ::declare("edge_double_map");
  declare_property_map<vector_property_map<int, EdgeIndexMap> >
    ::declare("edge_int_map");
  declare_property_map<vector_property_map<std::string, EdgeIndexMap> >
    ::declare("edge_string_map");
  declare_property_map<vector_property_map<object, EdgeIndexMap> >
    ::declare("edge_object_map");
  declare_property_map<vector_property_map<default_color_type, EdgeIndexMap> >
    ::declare("edge_color_map");
}

} } } // end namespace boost::graph::python
