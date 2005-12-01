// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Jeremiah Willcock
//           Douglas Gregor
//           Andrew Lumsdaine

// Compressed sparse row graph type

#ifndef BOOST_GRAPH_COMPRESSED_SPARSE_ROW_GRAPH_HPP
#define BOOST_GRAPH_COMPRESSED_SPARSE_ROW_GRAPH_HPP

#include <vector>
#include <utility>
#include <algorithm>
#include <climits>
#include <iterator>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/integer.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/mpl/if.hpp>

#ifdef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
#  error The Compressed Sparse Row graph only supports bundled properties.
#  error You will need a compiler that conforms better to the C++ standard.
#endif

namespace boost {

template<typename Derived, typename Property, typename Descriptor>
class indexed_vertex_properties
{
public:
  typedef no_property vertex_property_type;
  typedef Property vertex_bundled;

  // Directly access a vertex or edge bundle
  Property& operator[](Descriptor v)
  { return m_vertex_properties[get(vertex_index, derived(), v)]; }

  const Property& operator[](Descriptor v) const
  { return m_vertex_properties[get(vertex_index, derived(), v)]; }

protected:
  // Default-construct with no property values
  indexed_vertex_properties() {}

  // Initialize with n default-constructed property values
  indexed_vertex_properties(std::size_t n) : m_vertex_properties(n) { }

  // Resize the properties vector
  void resize(std::size_t n)
  {
    m_vertex_properties.resize(n);
  }

  // Reserve space in the vector of properties
  void reserve(std::size_t n)
  {
    m_vertex_properties.reserve(n);
  }

  // Add a new property value to the back
  void push_back(const Property& prop)
  {
    m_vertex_properties.push_back(prop);
  }

  // Access to the derived object
  Derived& derived() { return *static_cast<Derived*>(this); }

  const Derived& derived() const
  { return *static_cast<const Derived*>(this); }

public: // should be private, but friend templates not portable
  std::vector<Property> m_vertex_properties;
};

template<typename Derived, typename Descriptor>
class indexed_vertex_properties<Derived, void, Descriptor>
{
  struct secret {};

 public:
  typedef no_property vertex_property_type;
  typedef void vertex_bundled;

  secret operator[](secret) { return secret(); }

 protected:
  // All operations do nothing.
  indexed_vertex_properties() { }
  indexed_vertex_properties(std::size_t) { }
  void resize(std::size_t) { }
  void reserve(std::size_t) { }
};

template<typename Derived, typename Property, typename Descriptor>
class indexed_edge_properties
{
public:
  typedef no_property edge_property_type;
  typedef Property edge_bundled;

  // Directly access a edge or edge bundle
  Property& operator[](Descriptor v)
  { return m_edge_properties[get(edge_index, derived(), v)]; }

  const Property& operator[](Descriptor v) const
  { return m_edge_properties[get(edge_index, derived(), v)]; }

protected:
  // Default-construct with no property values
  indexed_edge_properties() {}

  // Initialize with n default-constructed property values
  indexed_edge_properties(std::size_t n) : m_edge_properties(n) { }

  // Resize the properties vector
  void resize(std::size_t n)
  {
    m_edge_properties.resize(n);
  }

  // Reserve space in the vector of properties
  void reserve(std::size_t n)
  {
    m_edge_properties.reserve(n);
  }

  // Add a new property value to the back
  void push_back(const Property& prop)
  {
    m_edge_properties.push_back(prop);
  }

  // Access to the derived object
  Derived& derived() { return *static_cast<Derived*>(this); }

  const Derived& derived() const
  { return *static_cast<const Derived*>(this); }

public: // should be private, but friend templates not portable
  std::vector<Property> m_edge_properties;
};

template<typename Derived, typename Descriptor>
class indexed_edge_properties<Derived, void, Descriptor>
{
  struct secret {};

 public:
  typedef no_property edge_property_type;
  typedef void edge_bundled;

  secret operator[](secret) { return secret(); }

 protected:
  // All operations do nothing.
  indexed_edge_properties() { }
  indexed_edge_properties(std::size_t) { }
  void resize(std::size_t) { }
  void reserve(std::size_t) { }
};

// A tag type indicating that the graph in question is a compressed
// sparse row graph. This is an internal detail of the BGL.
struct csr_graph_tag;

/****************************************************************************
 * Local helper macros to reduce typing and clutter later on.               *
 ****************************************************************************/
#define BOOST_CSR_GRAPH_TEMPLATE_PARMS                                  \
  typename VertexProperty, typename EdgeProperty, typename GraphProperty, \
  typename Vertex, typename EdgeIndex
#define BOOST_CSR_GRAPH_TYPE                                            \
   compressed_sparse_row_graph<VertexProperty, EdgeProperty, GraphProperty, \
                               Vertex, EdgeIndex>

// Forward declaration of CSR edge descriptor type, needed to pass to
// indexed_edge_properties.
template<typename Vertex, typename EdgeIndex>
class csr_edge_descriptor;

/** Compressed sparse row graph.
 *
 * Vertex and EdgeIndex should be unsigned integral types and should
 * specialize numeric_limits.
 */
template<typename VertexProperty = void,
         typename EdgeProperty = void,
         typename GraphProperty = no_property,
         typename Vertex = std::size_t,
         typename EdgeIndex = Vertex>
class compressed_sparse_row_graph
   : public indexed_vertex_properties<BOOST_CSR_GRAPH_TYPE, VertexProperty,
                                      Vertex>,
     public indexed_edge_properties<BOOST_CSR_GRAPH_TYPE, EdgeProperty,
                                    csr_edge_descriptor<Vertex, EdgeIndex> >

{
  typedef indexed_vertex_properties<compressed_sparse_row_graph,
                                    VertexProperty, Vertex>
    inherited_vertex_properties;

  typedef indexed_edge_properties<BOOST_CSR_GRAPH_TYPE, EdgeProperty,
                                  csr_edge_descriptor<Vertex, EdgeIndex> >
    inherited_edge_properties;

 public:
  // For Property Graph
  typedef GraphProperty graph_property_type;

 protected:
  template<typename InputIterator>
  void
  maybe_reserve_edge_list_storage(InputIterator, InputIterator,
                                  std::input_iterator_tag)
  {
    // Do nothing: we have no idea how much storage to reserve.
  }

  template<typename InputIterator>
  void
  maybe_reserve_edge_list_storage(InputIterator first, InputIterator last,
                                  std::forward_iterator_tag)
  {
    using std::distance;
    typename std::iterator_traits<InputIterator>::difference_type n =
      distance(first, last);
    m_column.reserve(n);
    inherited_edge_properties::reserve(n);
  }

 public:
  // Concept requirements:
  // For Graph
  typedef Vertex vertex_descriptor;
  typedef csr_edge_descriptor<Vertex, EdgeIndex> edge_descriptor;
  typedef directed_tag directed_category;
  typedef allow_parallel_edge_tag edge_parallel_category;

  class traversal_category: public incidence_graph_tag,
                            public adjacency_graph_tag,
                            public vertex_list_graph_tag,
                            public edge_list_graph_tag {};

  static vertex_descriptor null_vertex() { return vertex_descriptor(-1); }

  // For VertexListGraph
  typedef counting_iterator<Vertex> vertex_iterator;
  typedef Vertex vertices_size_type;

  // For EdgeListGraph
  typedef EdgeIndex edges_size_type;

  // For IncidenceGraph
  class out_edge_iterator;
  typedef EdgeIndex degree_size_type;

  // For AdjacencyGraph
  typedef typename std::vector<Vertex>::const_iterator adjacency_iterator;

  // For EdgeListGraph
  class edge_iterator;

  // For BidirectionalGraph (not implemented)
  typedef void in_edge_iterator;

  // For internal use
  typedef csr_graph_tag graph_tag;

  // Constructors

  // Default constructor: an empty graph.
  compressed_sparse_row_graph()
    : m_rowstart(1), m_column(0), m_property() {}

  //  From number of vertices and sorted list of edges
  template<typename InputIterator>
  compressed_sparse_row_graph(InputIterator edge_begin, InputIterator edge_end,
                              vertices_size_type numverts,
                              edges_size_type numedges = 0,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop)
  {
    // Reserving storage in advance can save us lots of time and
    // memory, but it can only be done if we have forward iterators or
    // the user has supplied the number of edges.
    if (numedges == 0) {
      typedef typename std::iterator_traits<InputIterator>::iterator_category
        category;
      maybe_reserve_edge_list_storage(edge_begin, edge_end, category());
    } else {
      m_column.reserve(numedges);
    }

    EdgeIndex current_edge = 0;
    Vertex current_vertex_plus_one = 1;
    m_rowstart[0] = 0;
    for (InputIterator ei = edge_begin; ei != edge_end; ++ei) {
      Vertex src = ei->first;
      Vertex tgt = ei->second;
      for (; current_vertex_plus_one != src + 1; ++current_vertex_plus_one)
        m_rowstart[current_vertex_plus_one] = current_edge;
      m_column.push_back(tgt);
      ++current_edge;
    }

    // The remaining vertices have no edges
    for (; current_vertex_plus_one != numverts + 1; ++current_vertex_plus_one)
      m_rowstart[current_vertex_plus_one] = current_edge;

    // Default-construct properties for edges
    inherited_edge_properties::resize(m_column.size());
  }

  //  From number of vertices and sorted list of edges
  template<typename InputIterator, typename EdgePropertyIterator>
  compressed_sparse_row_graph(InputIterator edge_begin, InputIterator edge_end,
                              EdgePropertyIterator ep_iter,
                              vertices_size_type numverts,
                              edges_size_type numedges = 0,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop)
  {
    // Reserving storage in advance can save us lots of time and
    // memory, but it can only be done if we have forward iterators or
    // the user has supplied the number of edges.
    if (numedges == 0) {
      typedef typename std::iterator_traits<InputIterator>::iterator_category
        category;
      maybe_reserve_edge_list_storage(edge_begin, edge_end, category());
    } else {
      m_column.reserve(numedges);
    }

    EdgeIndex current_edge = 0;
    Vertex current_vertex_plus_one = 1;
    m_rowstart[0] = 0;
    for (InputIterator ei = edge_begin; ei != edge_end; ++ei, ++ep_iter) {
      Vertex src = ei->first;
      Vertex tgt = ei->second;
      for (; current_vertex_plus_one != src + 1; ++current_vertex_plus_one)
        m_rowstart[current_vertex_plus_one] = current_edge;
      m_column.push_back(tgt);
      inherited_edge_properties::push_back(*ep_iter);
      ++current_edge;
    }

    // The remaining vertices have no edges
    for (; current_vertex_plus_one != numverts + 1; ++current_vertex_plus_one)
      m_rowstart[current_vertex_plus_one] = current_edge;
  }

  //   Requires IncidenceGraph, a vertex index map, and a vertex(n, g) function
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi,
                              vertices_size_type numverts,
                              edges_size_type numedges)
    : m_property()
  {
    assign(g, vi, numverts, numedges);
  }

  //   Requires VertexListGraph and EdgeListGraph
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi)
    : m_property()
  {
    assign(g, vi, num_vertices(g), num_edges(g));
  }

  // Requires vertex index map plus requirements of previous constructor
  template<typename Graph>
  explicit compressed_sparse_row_graph(const Graph& g)
    : m_property()
  {
    assign(g, get(vertex_index, g), num_vertices(g), num_edges(g));
  }

  // From any graph (slow and uses a lot of memory)
  //   Requires IncidenceGraph, a vertex index map, and a vertex(n, g) function
  //   Internal helper function
  template<typename Graph, typename VertexIndexMap>
  void
  assign(const Graph& g, const VertexIndexMap& vi,
         vertices_size_type numverts, edges_size_type numedges)
  {
    inherited_vertex_properties::resize(numverts);
    m_rowstart.resize(numverts + 1);
    m_column.resize(numedges);
    EdgeIndex current_edge = 0;
    typedef typename boost::graph_traits<Graph>::vertex_descriptor g_vertex;
    typedef typename boost::graph_traits<Graph>::edge_descriptor g_edge;
    typedef typename boost::graph_traits<Graph>::out_edge_iterator
      g_out_edge_iter;

    for (Vertex i = 0; i != numverts; ++i) {
      m_rowstart[i] = current_edge;
      g_vertex v = vertex(i, g);
      std::vector<Vertex> current_out_edges(out_degree(v, g));
      g_out_edge_iter ei, ei_end;
      EdgeIndex x = 0;
      for (tie(ei, ei_end) = out_edges(vertex(i, g), g); ei != ei_end; ++ei) {
        current_out_edges[x++] = get(vi, target(*ei, g));
      }
      std::sort(current_out_edges.begin(), current_out_edges.end());
      for (x = 0; x != current_out_edges.size(); ++x, ++current_edge)
        m_column[current_edge] = current_out_edges[x];
    }
    m_rowstart[numverts] = current_edge;
  }

  // Requires the above, plus VertexListGraph and EdgeListGraph
  template<typename Graph, typename VertexIndexMap>
  void assign(const Graph& g, const VertexIndexMap& vi)
  {
    assign(g, vi, num_vertices(g), num_edges(g));
  }

  // Requires the above, plus a vertex_index map.
  template<typename Graph>
  void assign(const Graph& g)
  {
    assign(g, get(vertex_index, g), num_vertices(g), num_edges(g));
  }

  using inherited_vertex_properties::operator[];
  using inherited_edge_properties::operator[];

  // private: non-portable, requires friend templates
  std::vector<EdgeIndex> m_rowstart;
  std::vector<Vertex> m_column;
  GraphProperty m_property;
};

template<typename Vertex, typename EdgeIndex>
class csr_edge_descriptor
{
 public:
  Vertex src;
  EdgeIndex idx;

  csr_edge_descriptor(Vertex src, EdgeIndex idx): src(src), idx(idx) {}
  csr_edge_descriptor(): src(0), idx(0) {}

  bool operator==(const csr_edge_descriptor& o) const {return idx == o.idx;}
  bool operator!=(const csr_edge_descriptor& o) const {return idx != o.idx;}
  bool operator<(const csr_edge_descriptor& e) const {return idx < e.idx;}
  bool operator>(const csr_edge_descriptor& e) const {return idx > e.idx;}
  bool operator<=(const csr_edge_descriptor& e) const {return idx <= e.idx;}
  bool operator>=(const csr_edge_descriptor& e) const {return idx >= e.idx;}
};

// From VertexListGraph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
num_vertices(const BOOST_CSR_GRAPH_TYPE& g) {
  return g.m_rowstart.size() - 1;
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
std::pair<counting_iterator<Vertex>, counting_iterator<Vertex> >
inline vertices(const BOOST_CSR_GRAPH_TYPE& g) {
  return std::make_pair(counting_iterator<Vertex>(0),
                        counting_iterator<Vertex>(num_vertices(g)));
}

// From IncidenceGraph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
class BOOST_CSR_GRAPH_TYPE::out_edge_iterator
  : public iterator_facade<out_edge_iterator,
                           edge_descriptor,
                           std::random_access_iterator_tag,
                           const edge_descriptor&,
                           typename int_t<CHAR_BIT * sizeof(EdgeIndex)>::fast>
{
 public:
  typedef typename int_t<CHAR_BIT * sizeof(EdgeIndex)>::fast difference_type;

  out_edge_iterator() {}
  // Implicit copy constructor OK
  explicit out_edge_iterator(edge_descriptor edge) : m_edge(edge) { }

 private:
  // iterator_facade requirements
  const edge_descriptor& dereference() const { return m_edge; }

  bool equal(const out_edge_iterator& other) const
  { return m_edge == other.m_edge; }

  void increment() { ++m_edge.idx; }
  void decrement() { ++m_edge.idx; }
  void advance(difference_type n) { m_edge.idx += n; }

  difference_type distance_to(const out_edge_iterator& other) const
  { return other.m_edge.idx - m_edge.idx; }

  edge_descriptor m_edge;

  friend class iterator_core_access;
};

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
source(typename BOOST_CSR_GRAPH_TYPE::edge_descriptor e,
       const BOOST_CSR_GRAPH_TYPE&)
{
  return e.src;
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
target(typename BOOST_CSR_GRAPH_TYPE::edge_descriptor e,
       const BOOST_CSR_GRAPH_TYPE& g)
{
  return g.m_column[e.idx];
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline std::pair<typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator,
                 typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator>
out_edges(Vertex v, const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_descriptor ed;
  typedef typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator it;
  return std::make_pair(it(ed(v, g.m_rowstart[v])),
                        it(ed(v, g.m_rowstart[v + 1])));
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline EdgeIndex
out_degree(Vertex v, const BOOST_CSR_GRAPH_TYPE& g)
{
  return g.m_rowstart[v + 1] - g.m_rowstart[v];
}

// From AdjacencyGraph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline std::pair<typename BOOST_CSR_GRAPH_TYPE::adjacency_iterator,
                 typename BOOST_CSR_GRAPH_TYPE::adjacency_iterator>
adjacent_vertices(Vertex v, const BOOST_CSR_GRAPH_TYPE& g)
{
  return std::make_pair(g.m_column.begin() + g.m_rowstart[v],
                        g.m_column.begin() + g.m_rowstart[v + 1]);
}

// Extra, common functions
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
vertex(Vertex i, const BOOST_CSR_GRAPH_TYPE&)
{
  return i;
}

// Unlike for an adjacency_matrix, edge_range and edge take lg(out_degree(i))
// time
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline std::pair<typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator,
                 typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator>
edge_range(Vertex i, Vertex j, const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename std::vector<Vertex>::const_iterator adj_iter;
  typedef typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator out_edge_iter;
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_descriptor edge_desc;
  std::pair<adj_iter, adj_iter> adjacencies =
    std::equal_range(
      g.m_column.begin() + g.m_rowstart[i],
      g.m_column.begin() + g.m_rowstart[i + 1],
      j);
  EdgeIndex idx_begin = adjacencies.first - g.m_column.begin();
  EdgeIndex idx_end = adjacencies.second - g.m_column.begin();
  return std::make_pair(out_edge_iter(edge_desc(i, idx_begin)),
                        out_edge_iter(edge_desc(i, idx_end)));
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline std::pair<typename BOOST_CSR_GRAPH_TYPE::edge_descriptor, bool>
edge(Vertex i, Vertex j, const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator out_edge_iter;
  std::pair<out_edge_iter, out_edge_iter> range = edge_range(i, j, g);
  if (range.first == range.second)
    return std::make_pair(typename BOOST_CSR_GRAPH_TYPE::edge_descriptor(),
                          false);
  else
    return std::make_pair(*range.first, true);
}

// Find an edge given its index in the graph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename BOOST_CSR_GRAPH_TYPE::edge_descriptor
edge_from_index(EdgeIndex idx, const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename std::vector<EdgeIndex>::const_iterator row_start_iter;
  row_start_iter src_plus_1 =
    std::upper_bound(g.m_rowstart.begin(), g.m_rowstart.end(), idx);
    // Get last source whose rowstart is at most idx
    // upper_bound returns this position plus 1
  Vertex src = (src_plus_1 - g.m_rowstart.begin()) - 1;
  return typename BOOST_CSR_GRAPH_TYPE::edge_descriptor(src, idx);
}

// From EdgeListGraph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
class BOOST_CSR_GRAPH_TYPE::edge_iterator
{
 public:
  typedef std::forward_iterator_tag iterator_category;
  typedef edge_descriptor value_type;

  class pointer
  {
  public:
    explicit pointer(const edge_descriptor& edge) : edge(edge) { }
    const edge_descriptor* operator->() const { return &edge; }

  private:
    edge_descriptor edge;
  };

  typedef edge_descriptor reference;
  typedef typename int_t<CHAR_BIT * sizeof(EdgeIndex)>::fast difference_type;

  edge_iterator() : rowstart_array(0), src(0), idx(0), end_of_this_vertex(0) {}

  edge_iterator(const compressed_sparse_row_graph& graph, Vertex src,
                EdgeIndex idx, EdgeIndex end_of_this_vertex)
    : rowstart_array(&graph.m_rowstart[0]), src(src), idx(idx),
      end_of_this_vertex(end_of_this_vertex) {}

  // From InputIterator
  reference operator*() const { return edge_descriptor(src, idx); }
  pointer operator->() const { return pointer(**this); }

  bool operator==(const edge_iterator& o) const {return idx == o.idx;}
  bool operator!=(const edge_iterator& o) const {return idx != o.idx;}

  edge_iterator& operator++() {
    ++idx;
    while (idx == end_of_this_vertex) {
      ++src;
      end_of_this_vertex = rowstart_array[src + 1];
    }
    return *this;
  }

  edge_iterator operator++(int) {
    edge_iterator temp = *this;
    ++*this;
    return temp;
  }

 private:
  const EdgeIndex* rowstart_array;
  Vertex src;
  EdgeIndex idx;
  EdgeIndex end_of_this_vertex;
};

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline EdgeIndex
num_edges(const BOOST_CSR_GRAPH_TYPE& g)
{
  return g.m_column.size();
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
std::pair<typename BOOST_CSR_GRAPH_TYPE::edge_iterator,
          typename BOOST_CSR_GRAPH_TYPE::edge_iterator>
edges(const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_iterator ei;
  if (g.m_rowstart.size() == 1) {
    return std::make_pair(ei(), ei());
  } else {
    // Find the first vertex that has outgoing edges
    Vertex src = 0;
    while (g.m_rowstart[src + 1] == 0) ++ src;
    return std::make_pair(ei(g, src, 0, g.m_rowstart[src + 1]),
                          ei(g, num_vertices(g), g.m_rowstart[num_vertices(g)],
                             0));
  }
}

// For Property Graph

// Graph properties
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, class Tag, class Value>
inline void
set_property(BOOST_CSR_GRAPH_TYPE& g, Tag, const Value& value)
{
  get_property_value(g.m_property, Tag()) = value;
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, class Tag>
inline
typename graph_property<BOOST_CSR_GRAPH_TYPE, Tag>::type&
get_property(BOOST_CSR_GRAPH_TYPE& g, Tag)
{
  return get_property_value(g.m_property, Tag());
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, class Tag>
inline
const
typename graph_property<BOOST_CSR_GRAPH_TYPE, Tag>::type&
get_property(const BOOST_CSR_GRAPH_TYPE& g, Tag)
{
  return get_property_value(g.m_property, Tag());
}

// Add edge_index property map
template<typename Index, typename Descriptor>
struct csr_edge_index_map
{
  typedef Index                     value_type;
  typedef Index                     reference;
  typedef Descriptor                key_type;
  typedef readable_property_map_tag category;
};

template<typename Index, typename Descriptor>
inline Index
get(const csr_edge_index_map<Index, Descriptor>&,
    const typename csr_edge_index_map<Index, Descriptor>::key_type& key)
{
  return key.idx;
}

// Doing the right thing here (by unifying with vertex_index_t and
// edge_index_t) breaks GCC.
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename Tag>
struct property_map<BOOST_CSR_GRAPH_TYPE, Tag>
{
private:
  typedef identity_property_map vertex_index_type;
  typedef typename graph_traits<BOOST_CSR_GRAPH_TYPE>::edge_descriptor
    edge_descriptor;
  typedef csr_edge_index_map<EdgeIndex, edge_descriptor> edge_index_type;

  typedef typename mpl::if_<is_same<Tag, edge_index_t>,
                            edge_index_type,
                            detail::error_property_not_found>::type
    edge_or_none;

public:
  typedef typename mpl::if_<is_same<Tag, vertex_index_t>,
                            vertex_index_type,
                            edge_or_none>::type type;

  typedef type const_type;
};

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline identity_property_map
get(vertex_index_t, const BOOST_CSR_GRAPH_TYPE&)
{
  return identity_property_map();
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
get(vertex_index_t,
    const BOOST_CSR_GRAPH_TYPE&, Vertex v)
{
  return v;
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename property_map<BOOST_CSR_GRAPH_TYPE, edge_index_t>::const_type
get(edge_index_t, const BOOST_CSR_GRAPH_TYPE&)
{
  typedef typename property_map<BOOST_CSR_GRAPH_TYPE, edge_index_t>::const_type
    result_type;
  return result_type();
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline EdgeIndex
get(edge_index_t, const BOOST_CSR_GRAPH_TYPE&,
    typename BOOST_CSR_GRAPH_TYPE::edge_descriptor e)
{
  return e.idx;
}

// Support for bundled properties
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename T, typename Bundle>
struct property_map<BOOST_CSR_GRAPH_TYPE, T Bundle::*>
{
private:
  typedef graph_traits<BOOST_CSR_GRAPH_TYPE> traits;
  typedef typename BOOST_CSR_GRAPH_TYPE::vertex_bundled vertex_bundled;
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_bundled edge_bundled;
  typedef typename ct_if<(detail::is_vertex_bundle<vertex_bundled, edge_bundled, Bundle>::value),
                     typename traits::vertex_descriptor,
                     typename traits::edge_descriptor>::type
    descriptor;

public:
  typedef bundle_property_map<BOOST_CSR_GRAPH_TYPE, descriptor, Bundle, T>
    type;
  typedef bundle_property_map<const BOOST_CSR_GRAPH_TYPE, descriptor, Bundle,
                              const T> const_type;
};

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename T, typename Bundle>
inline
typename property_map<BOOST_CSR_GRAPH_TYPE, T Bundle::*>::type
get(T Bundle::* p, BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename property_map<BOOST_CSR_GRAPH_TYPE,
                                T Bundle::*>::type
    result_type;
  return result_type(&g, p);
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS,
         typename Allocator, typename T, typename Bundle>
inline
typename property_map<BOOST_CSR_GRAPH_TYPE, T Bundle::*>::const_type
get(T Bundle::* p, BOOST_CSR_GRAPH_TYPE const & g)
{
  typedef typename property_map<BOOST_CSR_GRAPH_TYPE,
                                T Bundle::*>::const_type
    result_type;
  return result_type(&g, p);
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename T, typename Bundle,
         typename Key>
inline T
get(T Bundle::* p, BOOST_CSR_GRAPH_TYPE const & g,
    const Key& key)
{
  return get(get(p, g), key);
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename T, typename Bundle,
         typename Key>
inline void
put(T Bundle::* p, BOOST_CSR_GRAPH_TYPE& g,
    const Key& key, const T& value)
{
  put(get(p, g), key, value);
}

#undef BOOST_CSR_GRAPH_TYPE
#undef BOOST_CSR_GRAPH_TEMPLATE_PARMS

} // end namespace boost

#endif // BOOST_GRAPH_COMPRESSED_SPARSE_ROW_GRAPH_HPP
