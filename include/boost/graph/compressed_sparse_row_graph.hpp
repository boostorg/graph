// Copyright 2005-2006 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
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
#include <cassert>
#include <iterator>
#include <iostream> // FIXME
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/detail/indexed_properties.hpp>
#include <boost/iterator/counting_iterator.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/integer.hpp>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/mpl/if.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/static_assert.hpp>
#include <boost/functional/hash.hpp>
#include <boost/utility.hpp>

#ifdef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
#  error The Compressed Sparse Row graph only supports bundled properties.
#  error You will need a compiler that conforms better to the C++ standard.
#endif

namespace boost {

// A tag type indicating that the graph in question is a compressed
// sparse row graph. This is an internal detail of the BGL.
struct csr_graph_tag;

// A type (edges_are_sorted_t) and a value (edges_are_sorted) used to indicate
// that the edge list passed into the CSR graph is already sorted by source
// vertex.
struct edges_are_sorted_internal {};
inline void edges_are_sorted(edges_are_sorted_internal) {}
typedef void (*edges_are_sorted_t)(edges_are_sorted_internal);

// A type (construct_inplace_from_sources_and_targets_t) and a value
// (construct_inplace_from_sources_and_targets) used to indicate that mutable
// vectors of sources and targets (and possibly edge properties) are being used
// to construct the CSR graph.  These vectors are sorted in-place and then the
// targets and properties are swapped into the graph data structure.
struct construct_inplace_from_sources_and_targets_internal {};
inline void construct_inplace_from_sources_and_targets(construct_inplace_from_sources_and_targets_internal) {}
typedef void (*construct_inplace_from_sources_and_targets_t)(construct_inplace_from_sources_and_targets_internal);

// A type (construct_inplace_from_sources_and_targets_global_t) and a value
// (construct_inplace_from_sources_and_targets_global) used to indicate that
// mutable vectors of sources and targets (and possibly edge properties) are
// being used to construct the CSR graph.  These vectors are sorted in-place
// and then the targets and properties are swapped into the graph data
// structure.  It is assumed that global indices (for distributed CSR) are
// used, and a map is required to convert those to local indices.  This
// constructor is intended for internal use by the various CSR graphs
// (sequential and distributed).
struct construct_inplace_from_sources_and_targets_global_internal {};
inline void construct_inplace_from_sources_and_targets_global(construct_inplace_from_sources_and_targets_global_internal) {}
typedef void (*construct_inplace_from_sources_and_targets_global_t)(construct_inplace_from_sources_and_targets_global_internal);

/****************************************************************************
 * Local helper macros to reduce typing and clutter later on.               *
 ****************************************************************************/
#define BOOST_CSR_GRAPH_TEMPLATE_PARMS                                  \
  typename Directed, typename VertexProperty, typename EdgeProperty,    \
  typename GraphProperty, typename Vertex, typename EdgeIndex
#define BOOST_CSR_GRAPH_TYPE                                            \
   compressed_sparse_row_graph<Directed, VertexProperty, EdgeProperty,  \
                               GraphProperty, Vertex, EdgeIndex>

// Forward declaration of CSR edge descriptor type, needed to pass to
// indexed_edge_properties.
template<typename Vertex, typename EdgeIndex>
class csr_edge_descriptor;

namespace detail {
  // Less-than operator for comparing only the first elements of two arbitrary
  // Boost tuples
  struct compare_first_elements_in_tuples {
    template <typename Tuple>
    bool operator()(const Tuple& a, const Tuple& b) const {
      return (a.template get<0>()) < (b.template get<0>());
    }
  };
}

/** Compressed sparse row graph.
 *
 * Vertex and EdgeIndex should be unsigned integral types and should
 * specialize numeric_limits.
 */
template<typename Directed = directedS, 
         typename VertexProperty = void,
         typename EdgeProperty = void,
         typename GraphProperty = no_property,
         typename Vertex = std::size_t,
         typename EdgeIndex = Vertex>
class compressed_sparse_row_graph
   : public detail::indexed_vertex_properties<BOOST_CSR_GRAPH_TYPE, VertexProperty,
                                              Vertex>,
     public detail::indexed_edge_properties<BOOST_CSR_GRAPH_TYPE, EdgeProperty,
                                            csr_edge_descriptor<Vertex,
                                                                EdgeIndex> >

{
  typedef detail::indexed_vertex_properties<compressed_sparse_row_graph,
                                            VertexProperty, Vertex>
    inherited_vertex_properties;

  typedef detail::indexed_edge_properties<BOOST_CSR_GRAPH_TYPE, EdgeProperty,
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
  /* At this time, the compressed sparse row graph can only be used to
   * create a directed graph. In the future, bidirectional and
   * undirected CSR graphs will also be supported.
   */
  BOOST_STATIC_ASSERT((is_same<Directed, directedS>::value));

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
    : m_rowstart(1, EdgeIndex(0)), m_column(0), m_property(),
      m_last_source(0) {}

  //  With numverts vertices
  compressed_sparse_row_graph(vertices_size_type numverts)
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_last_source(numverts)
  {
    for (Vertex v = 0; v < numverts + 1; ++v)
      m_rowstart[v] = 0;
  }

  //  From number of vertices and unsorted list of edges
  template <typename MultiPassInputIterator>
  compressed_sparse_row_graph(MultiPassInputIterator edge_begin,
                              MultiPassInputIterator edge_end,
                              vertices_size_type numverts,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop), m_last_source(numverts)
  {
    // Put the degree of each vertex v into m_rowstart[v + 1]
    for (MultiPassInputIterator i = edge_begin; i != edge_end; ++i)
      ++m_rowstart[i->first + 1];

    // Compute the partial sum of the degrees to get the actual values of
    // m_rowstart
    EdgeIndex start_of_this_row = 0;
    m_rowstart[0] = start_of_this_row;
    for (vertices_size_type i = 1; i <= numverts; ++i) {
      start_of_this_row += m_rowstart[i];
      m_rowstart[i] = start_of_this_row;
    }
    m_column.resize(m_rowstart.back());

    // Histogram sort the edges by their source vertices, putting the targets
    // into m_column.  The index current_insert_positions[v] contains the next
    // location to insert out edges for vertex v.
    std::vector<EdgeIndex>
      current_insert_positions(m_rowstart.begin(), m_rowstart.begin() + numverts);
    for (; edge_begin != edge_end; ++edge_begin)
      m_column[current_insert_positions[edge_begin->first]++] = edge_begin->second;

    // Default-construct properties for edges
    inherited_edge_properties::resize(m_column.size());
  }

  //  From number of vertices and unsorted list of edges, plus edge properties
  template <typename MultiPassInputIterator, typename EdgePropertyIterator>
  compressed_sparse_row_graph(MultiPassInputIterator edge_begin,
                              MultiPassInputIterator edge_end,
                              EdgePropertyIterator ep_iter,
                              vertices_size_type numverts,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop), m_last_source(numverts)
  {
    // Put the degree of each vertex v into m_rowstart[v + 1]
    for (MultiPassInputIterator i = edge_begin; i != edge_end; ++i)
      ++m_rowstart[i->first + 1];

    // Compute the partial sum of the degrees to get the actual values of
    // m_rowstart
    EdgeIndex start_of_this_row = 0;
    m_rowstart[0] = start_of_this_row;
    for (vertices_size_type i = 1; i <= numverts; ++i) {
      start_of_this_row += m_rowstart[i];
      m_rowstart[i] = start_of_this_row;
    }
    m_column.resize(m_rowstart.back());
    inherited_edge_properties::resize(m_rowstart.back());

    // Histogram sort the edges by their source vertices, putting the targets
    // into m_column.  The index current_insert_positions[v] contains the next
    // location to insert out edges for vertex v.
    std::vector<EdgeIndex>
      current_insert_positions(m_rowstart.begin(), m_rowstart.begin() + numverts);
    for (; edge_begin != edge_end; ++edge_begin, ++ep_iter) {
      vertices_size_type source = edge_begin->first;
      EdgeIndex insert_pos = current_insert_positions[source];
      ++current_insert_positions[source];
      m_column[insert_pos] = edge_begin->second;
      inherited_edge_properties::write_by_index(insert_pos, *ep_iter);
    }
  }

  //  From number of vertices and sorted list of edges
  template<typename InputIterator>
  compressed_sparse_row_graph(edges_are_sorted_t,
                              InputIterator edge_begin, InputIterator edge_end,
                              vertices_size_type numverts,
                              edges_size_type numedges = 0,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop), m_last_source(numverts)
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
  compressed_sparse_row_graph(edges_are_sorted_t,
                              InputIterator edge_begin, InputIterator edge_end,
                              EdgePropertyIterator ep_iter,
                              vertices_size_type numverts,
                              edges_size_type numedges = 0,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(numverts + 1),
      m_column(0), m_property(prop), m_last_source(numverts)
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

  //  From number of vertices and mutable vectors of sources and targets;
  //  vectors are returned with unspecified contents but are guaranteed not to
  //  share storage with the constructed graph.
  compressed_sparse_row_graph(construct_inplace_from_sources_and_targets_t,
                              std::vector<vertex_descriptor>& sources,
                              std::vector<vertex_descriptor>& targets,
                              vertices_size_type numverts,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(),
      m_column(), m_property(prop), m_last_source(numverts)
  {
    assign_sources_and_targets_global(sources, targets, numverts, boost::identity_property_map());
  }

  //  From number of vertices and mutable vectors of sources and targets,
  //  expressed with global vertex indices; vectors are returned with
  //  unspecified contents but are guaranteed not to share storage with the
  //  constructed graph.  This constructor should only be used by the
  //  distributed CSR graph.
  template <typename GlobalToLocal>
  compressed_sparse_row_graph(construct_inplace_from_sources_and_targets_global_t,
                              std::vector<vertex_descriptor>& sources,
                              std::vector<vertex_descriptor>& targets,
                              vertices_size_type numlocalverts,
                              GlobalToLocal global_to_local,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numlocalverts), m_rowstart(),
      m_column(), m_property(prop), m_last_source(numlocalverts)
  {
    assign_sources_and_targets_global(sources, targets, numlocalverts, global_to_local);
  }

  //  From number of vertices and mutable vectors of sources, targets, and edge
  //  properties; vectors are returned with unspecified contents but are
  //  guaranteed not to share storage with the constructed graph.
  compressed_sparse_row_graph(construct_inplace_from_sources_and_targets_t,
                              std::vector<vertex_descriptor>& sources,
                              std::vector<vertex_descriptor>& targets,
                              std::vector<typename inherited_edge_properties::edge_property_type>& edge_props,
                              vertices_size_type numverts,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numverts), m_rowstart(),
      m_column(), m_property(prop), m_last_source(numverts)
  {
    assign_sources_and_targets_global(sources, targets, edge_props, numverts, boost::identity_property_map());
  }

  //  From number of vertices and mutable vectors of sources and targets and
  //  edge properties, expressed with global vertex indices; vectors are
  //  returned with unspecified contents but are guaranteed not to share
  //  storage with the constructed graph.  This constructor should only be used
  //  by the distributed CSR graph.
  template <typename GlobalToLocal>
  compressed_sparse_row_graph(construct_inplace_from_sources_and_targets_global_t,
                              std::vector<vertex_descriptor>& sources,
                              std::vector<vertex_descriptor>& targets,
                              std::vector<typename inherited_edge_properties::edge_property_type>& edge_props,
                              vertices_size_type numlocalverts,
                              GlobalToLocal global_to_local,
                              const GraphProperty& prop = GraphProperty())
    : inherited_vertex_properties(numlocalverts), m_rowstart(),
      m_column(), m_property(prop), m_last_source(numlocalverts)
  {
    assign_sources_and_targets_global(sources, targets, edge_props, numlocalverts, global_to_local);
  }

  // Replace graph with sources and targets given, sorting them in-place, and
  // using the given global-to-local property map to get local indices from
  // global ones in the two arrays.
  template <typename GlobalToLocal>
  void assign_sources_and_targets_global(std::vector<vertex_descriptor>& sources,
                                         std::vector<vertex_descriptor>& targets,
                                         vertices_size_type numverts,
                                         GlobalToLocal global_to_local) {
    assert (sources.size() == targets.size());
    typedef typename std::vector<vertex_descriptor>::iterator vertex_vec_iter;
    EdgeIndex numedges = sources.size();
#if 0
    std::cerr << "Edges before:";
    for (size_t i = 0; i < numedges; ++i) {
      std::cerr << " (" << sources[i] << " -> " << targets[i] << ")";
    }
    std::cerr << std::endl;
#endif
    // Do an in-place histogram sort (at least that's what I think it is) to
    // sort sources and targets
    // 1. Count degrees; degree of vertex v is in m_rowstart[v + 1]
    m_rowstart.clear();
    m_rowstart.resize(numverts + 1);
    for (size_t i = 0; i < numedges; ++i) {
      ++m_rowstart[get(global_to_local, sources[i]) + 1];
    }
    // 2. Do a prefix sum on those to get starting positions of each row
    for (size_t i = 0; i < numverts; ++i) {
      m_rowstart[i + 1] += m_rowstart[i];
    }
    // 3. Copy m_rowstart (except last element) to get insert positions
    std::vector<EdgeIndex> insert_positions(m_rowstart.begin(), boost::prior(m_rowstart.end()));
    // 4. Swap the sources and targets into place
    for (size_t i = 0; i < numedges; ++i) {
      // While edge i is not in the right bucket:
      while (!(i >= m_rowstart[get(global_to_local, sources[i])] && i < insert_positions[get(global_to_local, sources[i])])) {
        // Add a slot in the right bucket
        size_t target_pos = insert_positions[get(global_to_local, sources[i])]++;
        assert (target_pos < m_rowstart[get(global_to_local, sources[i]) + 1]);
        if (target_pos == i) continue;
        // Swap this edge into place
        using std::swap;
        swap(sources[i], sources[target_pos]);
        swap(targets[i], targets[target_pos]);
      }
    }
#if 0
    std::cerr << "Edges after:";
    for (size_t i = 0; i < sources.size(); ++i) {
      std::cerr << " (" << sources[i] << " -> " << targets[i] << ")";
    }
    std::cerr << std::endl;
#endif
    // Now targets is the correct vector (properly sorted by source) for
    // m_column
    m_column.swap(targets);
  }

  // Replace graph with sources and targets and edge properties given, sorting
  // them in-place, and using the given global-to-local property map to get
  // local indices from global ones in the two arrays.
  template <typename GlobalToLocal>
  void assign_sources_and_targets_global(std::vector<vertex_descriptor>& sources,
                                         std::vector<vertex_descriptor>& targets,
                                         std::vector<typename inherited_edge_properties::edge_property_type>& edge_props,
                                         vertices_size_type numverts,
                                         GlobalToLocal global_to_local) {
    assert (sources.size() == targets.size());
    assert (sources.size() == edge_props.size());
    EdgeIndex numedges = sources.size();
#if 0
    std::cerr << "Edges before:";
    for (size_t i = 0; i < numedges; ++i) {
      std::cerr << " (" << sources[i] << " -> " << targets[i] << ")";
    }
    std::cerr << std::endl;
#endif
    // Do an in-place histogram sort (at least that's what I think it is) to
    // sort sources and targets
    // 1. Count degrees; degree of vertex v is in m_rowstart[v + 1]
    m_rowstart.clear();
    m_rowstart.resize(numverts + 1);
    for (size_t i = 0; i < numedges; ++i) {
      ++m_rowstart[get(global_to_local, sources[i]) + 1];
    }
    // 2. Do a prefix sum on those to get starting positions of each row
    for (size_t i = 0; i < numverts; ++i) {
      m_rowstart[i + 1] += m_rowstart[i];
    }
    // 3. Copy m_rowstart (except last element) to get insert positions
    std::vector<EdgeIndex> insert_positions(m_rowstart.begin(), boost::prior(m_rowstart.end()));
    // 4. Swap the sources and targets into place
    for (size_t i = 0; i < numedges; ++i) {
      // While edge i is not in the right bucket:
      while (!(i >= m_rowstart[get(global_to_local, sources[i])] && i < insert_positions[get(global_to_local, sources[i])])) {
        // Add a slot in the right bucket
        size_t target_pos = insert_positions[get(global_to_local, sources[i])]++;
        assert (target_pos < m_rowstart[get(global_to_local, sources[i]) + 1]);
        if (target_pos == i) continue;
        // Swap this edge into place
        using std::swap;
        swap(sources[i], sources[target_pos]);
        swap(targets[i], targets[target_pos]);
        swap(edge_props[i], edge_props[target_pos]);
      }
    }
#if 0
    std::cerr << "Edges after:";
    for (size_t i = 0; i < sources.size(); ++i) {
      std::cerr << " (" << sources[i] << " -> " << targets[i] << ")";
    }
    std::cerr << std::endl;
#endif
    // Now targets is the correct vector (properly sorted by source) for
    // m_column, and edge_props for m_edge_properties
    m_column.swap(targets);
    this->m_edge_properties.swap(edge_props);
  }


  //   Requires IncidenceGraph, a vertex index map, and a vertex(n, g) function
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi,
                              vertices_size_type numverts,
                              edges_size_type numedges)
    : m_property(), m_last_source(0)
  {
    assign(g, vi, numverts, numedges);
  }

  //   Requires VertexListGraph and EdgeListGraph
  template<typename Graph, typename VertexIndexMap>
  compressed_sparse_row_graph(const Graph& g, const VertexIndexMap& vi)
    : m_property(), m_last_source(0)
  {
    assign(g, vi, num_vertices(g), num_edges(g));
  }

  // Requires vertex index map plus requirements of previous constructor
  template<typename Graph>
  explicit compressed_sparse_row_graph(const Graph& g)
    : m_property(), m_last_source(0)
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
      g_out_edge_iter ei, ei_end;
      for (tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
        m_column[current_edge++] = get(vi, target(*ei, g));
      }
    }
    m_rowstart[numverts] = current_edge;
    m_last_source = numverts;
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
  inherited_vertex_properties&       vertex_properties()       {return *this;}
  const inherited_vertex_properties& vertex_properties() const {return *this;}
  inherited_edge_properties&       edge_properties()       { return *this; }
  const inherited_edge_properties& edge_properties() const { return *this; }

  std::vector<EdgeIndex> m_rowstart;
  std::vector<Vertex> m_column;
  GraphProperty m_property;
  Vertex m_last_source; // Last source of added edge, plus one
};

template<typename Vertex, typename EdgeIndex>
class csr_edge_descriptor
{
 public:
  Vertex src;
  EdgeIndex idx;

  csr_edge_descriptor(Vertex src, EdgeIndex idx): src(src), idx(idx) {}
  csr_edge_descriptor(): src(0), idx(0) {}

  bool operator==(const csr_edge_descriptor& e) const {return idx == e.idx;}
  bool operator!=(const csr_edge_descriptor& e) const {return idx != e.idx;}
  bool operator<(const csr_edge_descriptor& e) const {return idx < e.idx;}
  bool operator>(const csr_edge_descriptor& e) const {return idx > e.idx;}
  bool operator<=(const csr_edge_descriptor& e) const {return idx <= e.idx;}
  bool operator>=(const csr_edge_descriptor& e) const {return idx >= e.idx;}

  template<typename Archiver>
  void serialize(Archiver& ar, const unsigned int /*version*/)
  {
    ar & src & idx;
  }
};

template<typename Vertex, typename EdgeIndex>
struct hash<csr_edge_descriptor<Vertex, EdgeIndex> >
{
  std::size_t operator()(csr_edge_descriptor<Vertex, EdgeIndex> const& x) const
  {
    std::size_t hash = hash_value(x.src);
    hash_combine(hash, x.idx);
    return hash;
  }
};

// Construction functions
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
add_vertex(BOOST_CSR_GRAPH_TYPE& g) {
  Vertex old_num_verts_plus_one = g.m_rowstart.size();
  g.m_rowstart.push_back(EdgeIndex(0));
  g.vertex_properties().resize(num_vertices(g));
  return old_num_verts_plus_one - 1;
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline Vertex
add_vertices(typename BOOST_CSR_GRAPH_TYPE::vertices_size_type count, BOOST_CSR_GRAPH_TYPE& g) {
  Vertex old_num_verts_plus_one = g.m_rowstart.size();
  g.m_rowstart.resize(old_num_verts_plus_one + count, EdgeIndex(0));
  g.vertex_properties().resize(num_vertices(g));
  return old_num_verts_plus_one - 1;
}

// This function requires that src be at least as large as the largest source
// in the graph so far
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename BOOST_CSR_GRAPH_TYPE::edge_descriptor
add_edge(Vertex src, Vertex tgt, BOOST_CSR_GRAPH_TYPE& g) {
  assert ((g.m_last_source == 0 || src >= g.m_last_source - 1) &&
          src < num_vertices(g));
  EdgeIndex num_edges_orig = g.m_column.size();
  for (; g.m_last_source <= src; ++g.m_last_source)
    g.m_rowstart[g.m_last_source] = num_edges_orig;
  g.m_rowstart[src + 1] = num_edges_orig + 1;
  g.m_column.push_back(tgt);
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_push_back_type push_back_type;
  g.edge_properties().push_back(push_back_type());
  return typename BOOST_CSR_GRAPH_TYPE::edge_descriptor(src, num_edges_orig);
}

// This function requires that src be at least as large as the largest source
// in the graph so far
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename BOOST_CSR_GRAPH_TYPE::edge_descriptor
add_edge(Vertex src, Vertex tgt,
         typename BOOST_CSR_GRAPH_TYPE::edge_bundled const& p,
         BOOST_CSR_GRAPH_TYPE& g) {
  assert ((g.m_last_source == 0 || src >= g.m_last_source - 1) &&
          src < num_vertices(g));
  EdgeIndex num_edges_orig = g.m_column.size();
  for (; g.m_last_source <= src; ++g.m_last_source)
    g.m_rowstart[g.m_last_source] = num_edges_orig;
  g.m_rowstart[src + 1] = num_edges_orig + 1;
  g.m_column.push_back(tgt);
  g.edge_properties().push_back(p);
  return typename BOOST_CSR_GRAPH_TYPE::edge_descriptor(src, num_edges_orig);
}


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
  : public iterator_facade<typename BOOST_CSR_GRAPH_TYPE::out_edge_iterator,
                           typename BOOST_CSR_GRAPH_TYPE::edge_descriptor,
                           std::random_access_iterator_tag,
                           const typename BOOST_CSR_GRAPH_TYPE::edge_descriptor&,
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
  EdgeIndex v_row_start = g.m_rowstart[v];
  EdgeIndex next_row_start = g.m_rowstart[v + 1];
  return std::make_pair(it(ed(v, v_row_start)),
                        it(ed(v, (std::max)(v_row_start, next_row_start))));
}

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline EdgeIndex
out_degree(Vertex v, const BOOST_CSR_GRAPH_TYPE& g)
{
  EdgeIndex v_row_start = g.m_rowstart[v];
  EdgeIndex next_row_start = g.m_rowstart[v + 1];
  return (std::max)(v_row_start, next_row_start) - v_row_start;
}

// From AdjacencyGraph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline std::pair<typename BOOST_CSR_GRAPH_TYPE::adjacency_iterator,
                 typename BOOST_CSR_GRAPH_TYPE::adjacency_iterator>
adjacent_vertices(Vertex v, const BOOST_CSR_GRAPH_TYPE& g)
{
  EdgeIndex v_row_start = g.m_rowstart[v];
  EdgeIndex next_row_start = g.m_rowstart[v + 1];
  return std::make_pair(g.m_column.begin() + v_row_start,
                        g.m_column.begin() +
                                (std::max)(v_row_start, next_row_start));
}

// Extra, common functions
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename graph_traits<BOOST_CSR_GRAPH_TYPE>::vertex_descriptor
vertex(typename graph_traits<BOOST_CSR_GRAPH_TYPE>::vertex_descriptor i, 
       const BOOST_CSR_GRAPH_TYPE&)
{
  return i;
}

// Find an edge given its index in the graph
template<BOOST_CSR_GRAPH_TEMPLATE_PARMS>
inline typename BOOST_CSR_GRAPH_TYPE::edge_descriptor
edge_from_index(EdgeIndex idx, const BOOST_CSR_GRAPH_TYPE& g)
{
  typedef typename std::vector<EdgeIndex>::const_iterator row_start_iter;
  assert (idx < num_edges(g));
  row_start_iter src_plus_1 =
    std::upper_bound(g.m_rowstart.begin(),
                     g.m_rowstart.begin() + g.m_last_source + 1,
                     idx);
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

  typedef const edge_descriptor* pointer;

  typedef edge_descriptor reference;
  typedef typename int_t<CHAR_BIT * sizeof(EdgeIndex)>::fast difference_type;

  edge_iterator() : rowstart_array(0), current_edge(), end_of_this_vertex(0) {}

  edge_iterator(const compressed_sparse_row_graph& graph,
                edge_descriptor current_edge,
                EdgeIndex end_of_this_vertex)
    : rowstart_array(&graph.m_rowstart[0]), current_edge(current_edge),
      end_of_this_vertex(end_of_this_vertex) {}

  // From InputIterator
  reference operator*() const { return current_edge; }
  pointer operator->() const { return &current_edge; }

  bool operator==(const edge_iterator& o) const {
    return current_edge == o.current_edge;
  }
  bool operator!=(const edge_iterator& o) const {
    return current_edge != o.current_edge;
  }

  edge_iterator& operator++() {
    ++current_edge.idx;
    while (current_edge.idx == end_of_this_vertex) {
      ++current_edge.src;
      end_of_this_vertex = rowstart_array[current_edge.src + 1];
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
  edge_descriptor current_edge;
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
  typedef typename BOOST_CSR_GRAPH_TYPE::edge_descriptor edgedesc;
  if (g.m_rowstart.size() == 1 || g.m_column.empty()) {
    return std::make_pair(ei(), ei());
  } else {
    // Find the first vertex that has outgoing edges
    Vertex src = 0;
    while (g.m_rowstart[src + 1] == 0) ++src;
    return std::make_pair(ei(g, edgedesc(src, 0), g.m_rowstart[src + 1]),
                          ei(g, edgedesc(num_vertices(g), g.m_column.size()), 0));
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
  typedef VertexProperty vertex_bundled;
  typedef EdgeProperty edge_bundled;
  typedef typename mpl::if_c<(detail::is_vertex_bundle<vertex_bundled, edge_bundled, Bundle>::value),
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

template<BOOST_CSR_GRAPH_TEMPLATE_PARMS, typename T, typename Bundle>
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
