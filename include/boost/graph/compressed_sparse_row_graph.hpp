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

namespace boost {

// A tag type indicating that the graph in question is a compressed
// sparse row graph. This is an internal detail of the BGL.
struct csr_graph_tag;

/** Compressed sparse row graph.
 *
 * Vertex and EdgeIndex should be unsigned integral types and should
 * specialize numeric_limits.
 *
 * DPG TBD: Shall we take vertex/edge properties?
 */
template<typename Vertex = std::size_t, typename EdgeIndex = Vertex>
class compressed_sparse_row_graph
{
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
    m_column.reserve(distance(first, last));
  }

 public:
  // Concept requirements:
  // For Graph
  typedef Vertex vertex_descriptor;
  class edge_descriptor;
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
  typedef no_property vertex_property_type;
  typedef no_property edge_property_type;
  typedef csr_graph_tag graph_tag;

  // Constructors

  // Default constructor: an empty graph.
  compressed_sparse_row_graph(): m_rowstart(1), m_column(0) {}

  //  From number of vertices and sorted list of edges
  template<typename InputIterator>
  compressed_sparse_row_graph(InputIterator edge_begin, InputIterator edge_end,
                              vertices_size_type numverts,
                              edges_size_type numedges = 0)
    : m_rowstart(numverts + 1), m_column(0)
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
  }

  //   Requires IncidenceGraph, a vertex index map, and a vertex(n, g) function
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi,
                              vertices_size_type numverts,
                              edges_size_type numedges)
  {
    assign(g, vi, numverts, numedges);
  }

  //   Requires VertexListGraph and EdgeListGraph
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi)
  {
    assign(g, vi, num_vertices(g), num_edges(g));
  }

  // Requires vertex index map plus requirements of previous constructor
  template<typename Graph>
  explicit compressed_sparse_row_graph(const Graph& g)
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

  // private: non-portable, requires friend templates
  std::vector<EdgeIndex> m_rowstart;
  std::vector<Vertex> m_column;
};

template<typename Vertex, typename EdgeIndex>
class compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_descriptor
{
 public:
  Vertex src;
  EdgeIndex idx;

  edge_descriptor(Vertex src, EdgeIndex idx): src(src), idx(idx) {}

  edge_descriptor(): src(0), idx(0) {}

  bool operator==(const edge_descriptor& o) const {return idx == o.idx;}
  bool operator!=(const edge_descriptor& o) const {return idx != o.idx;}
  bool operator<(const edge_descriptor& e) const {return idx < e.idx;}
  bool operator>(const edge_descriptor& e) const {return idx > e.idx;}
  bool operator<=(const edge_descriptor& e) const {return idx <= e.idx;}
  bool operator>=(const edge_descriptor& e) const {return idx >= e.idx;}
};

// From VertexListGraph
template<typename Vertex, typename EdgeIndex>
inline Vertex
num_vertices(const compressed_sparse_row_graph<Vertex, EdgeIndex>& g) {
  return g.m_rowstart.size() - 1;
}

template<typename Vertex, typename EdgeIndex>
std::pair<counting_iterator<Vertex>, counting_iterator<Vertex> >
inline vertices(const compressed_sparse_row_graph<Vertex, EdgeIndex>& g) {
  return std::make_pair(counting_iterator<Vertex>(0),
                        counting_iterator<Vertex>(num_vertices(g)));
}

// From IncidenceGraph
template<typename Vertex, typename EdgeIndex>
class compressed_sparse_row_graph<Vertex, EdgeIndex>::out_edge_iterator
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

template<typename Vertex, typename EdgeIndex>
inline Vertex
source(typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_descriptor e,
       const compressed_sparse_row_graph<Vertex, EdgeIndex>&)
{
  return e.src;
}

template<typename Vertex, typename EdgeIndex>
inline Vertex
target(typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_descriptor e,
       const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  return g.m_column[e.idx];
}

template<typename Vertex, typename EdgeIndex>
inline std::pair<typename compressed_sparse_row_graph<Vertex, EdgeIndex>::out_edge_iterator,
                 typename compressed_sparse_row_graph<Vertex, EdgeIndex>::out_edge_iterator>
out_edges(Vertex v, const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  typedef typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_descriptor ed;
  typedef typename compressed_sparse_row_graph<Vertex, EdgeIndex>::out_edge_iterator it;
  return std::make_pair(it(ed(v, g.m_rowstart[v])),
                        it(ed(v, g.m_rowstart[v + 1])));
}

template<typename Vertex, typename EdgeIndex>
inline EdgeIndex
out_degree(Vertex v, const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  return g.m_rowstart[v + 1] - g.m_rowstart[v];
}

// From AdjacencyGraph
template<typename Vertex, typename EdgeIndex>
inline std::pair<typename compressed_sparse_row_graph<Vertex, EdgeIndex>::adjacency_iterator,
                 typename compressed_sparse_row_graph<Vertex, EdgeIndex>::adjacency_iterator>
adjacent_vertices(Vertex v, const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  return std::make_pair(g.m_column.begin() + g.m_rowstart[v],
                        g.m_column.begin() + g.m_rowstart[v + 1]);
}

// Extra, common function
template<typename Vertex, typename EdgeIndex>
inline Vertex
vertex(Vertex i, const compressed_sparse_row_graph<Vertex, EdgeIndex>&)
{
  return i;
}

// From EdgeListGraph
template<typename Vertex, typename EdgeIndex>
class compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_iterator
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

template<typename Vertex, typename EdgeIndex>
inline EdgeIndex
num_edges(const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  return g.m_column.size();
}

template<typename Vertex, typename EdgeIndex>
std::pair<typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_iterator,
          typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_iterator>
edges(const compressed_sparse_row_graph<Vertex, EdgeIndex>& g)
{
  typedef typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_iterator ei;
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

// Add vertex_index and edge_index property maps
template<typename Vertex, typename EdgeIndex>
struct csr_edge_index_map
{
  typedef EdgeIndex value_type;
  typedef EdgeIndex reference;
  typedef typename compressed_sparse_row_graph<Vertex, EdgeIndex>
                     ::edge_descriptor key_type;
  typedef readable_property_map_tag category;
};

template<typename Vertex, typename EdgeIndex>
inline EdgeIndex
get(const csr_edge_index_map<Vertex, EdgeIndex>&,
    const typename csr_edge_index_map<Vertex, EdgeIndex>::key_type& key)
{
  return key.idx;
}

// Doing the right thing here (by unifying with vertex_index_t and
// edge_index_t) breaks GCC.
template<typename Vertex, typename EdgeIndex, typename Tag>
struct property_map<compressed_sparse_row_graph<Vertex, EdgeIndex>, Tag>
{
private:
  typedef identity_property_map vertex_index_type;
  typedef csr_edge_index_map<Vertex, EdgeIndex> edge_index_type;

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

template<typename Vertex, typename EdgeIndex>
inline identity_property_map
get(vertex_index_t, const compressed_sparse_row_graph<Vertex, EdgeIndex>&)
{
  return identity_property_map();
}

template<typename Vertex, typename EdgeIndex>
inline Vertex
get(vertex_index_t,
    const compressed_sparse_row_graph<Vertex, EdgeIndex>&, Vertex v)
{
  return v;
}

template<typename Vertex, typename EdgeIndex>
inline csr_edge_index_map<Vertex, EdgeIndex>
get(edge_index_t, const compressed_sparse_row_graph<Vertex, EdgeIndex>&)
{
  return csr_edge_index_map<Vertex, EdgeIndex>();
}

template<typename Vertex, typename EdgeIndex>
inline EdgeIndex
get(edge_index_t, const compressed_sparse_row_graph<Vertex, EdgeIndex>&,
    typename compressed_sparse_row_graph<Vertex, EdgeIndex>::edge_descriptor e)
{
  return e.idx;
}

} // end namespace boost

#endif // BOOST_GRAPH_COMPRESSED_SPARSE_ROW_GRAPH_HPP
