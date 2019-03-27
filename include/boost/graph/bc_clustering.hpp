// Copyright 2004 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP
#define BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP

#include <boost/graph/betweenness_centrality.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <algorithm>
#include <vector>
#include <boost/property_map/property_map.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/parameter/preprocessor.hpp>
#endif

namespace boost {

/** Threshold termination function for the betweenness centrality
 * clustering algorithm.
 */
template<typename T>
struct bc_clustering_threshold
{
  typedef T centrality_type;

  /// Terminate clustering when maximum absolute edge centrality is
  /// below the given threshold.
  explicit bc_clustering_threshold(T threshold) 
    : threshold(threshold), dividend(1.0) {}
  
  /**
   * Terminate clustering when the maximum edge centrality is below
   * the given threshold.
   *
   * @param threshold the threshold value
   *
   * @param g the graph on which the threshold will be calculated
   *
   * @param normalize when true, the threshold is compared against the
   * normalized edge centrality based on the input graph; otherwise,
   * the threshold is compared against the absolute edge centrality.
   */
  template<typename Graph>
  bc_clustering_threshold(T threshold, const Graph& g, bool normalize = true)
    : threshold(threshold), dividend(1.0)
  {
    if (normalize) {
      typename graph_traits<Graph>::vertices_size_type n = num_vertices(g);
      dividend = T((n - 1) * (n - 2)) / T(2);
    }
  }

  /** Returns true when the given maximum edge centrality (potentially
   * normalized) falls below the threshold.
   */
  template<typename Graph, typename Edge>
  bool operator()(T max_centrality, Edge, const Graph&) const
  {
    return (max_centrality / dividend) < threshold;
  }

 protected:
  T threshold;
  T dividend;
};

/** Graph clustering based on edge betweenness centrality.
 * 
 * This algorithm implements graph clustering based on edge
 * betweenness centrality. It is an iterative algorithm, where in each
 * step it compute the edge betweenness centrality (via @ref
 * brandes_betweenness_centrality) and removes the edge with the
 * maximum betweenness centrality. The @p done function object
 * determines when the algorithm terminates (the edge found when the
 * algorithm terminates will not be removed).
 *
 * @param g The graph on which clustering will be performed. The type
 * of this parameter (@c MutableGraph) must be a model of the
 * VertexListGraph, IncidenceGraph, EdgeListGraph, and Mutable Graph
 * concepts.
 *
 * @param done The function object that indicates termination of the
 * algorithm. It must be a ternary function object thats accepts the
 * maximum centrality, the descriptor of the edge that will be
 * removed, and the graph @p g.
 *
 * @param edge_centrality (UTIL/OUT) The property map that will store
 * the betweenness centrality for each edge. When the algorithm
 * terminates, it will contain the edge centralities for the
 * graph. The type of this property map must model the
 * ReadWritePropertyMap concept. Defaults to an @c
 * iterator_property_map whose value type is 
 * @c Done::centrality_type and using @c get(edge_index, g) for the 
 * index map.
 *
 * @param vertex_index (IN) The property map that maps vertices to
 * indices in the range @c [0, num_vertices(g)). This type of this
 * property map must model the ReadablePropertyMap concept and its
 * value type must be an integral type. Defaults to 
 * @c get(vertex_index, g).
 */
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
BOOST_PARAMETER_FUNCTION(
    (bool), betweenness_centrality_clustering, ::boost::graph::keywords::tag,
    (required
        (graph, *)
    )
    (deduced
        (required
            (terminator_function
              ,*(detail::clustering_terminator_function_predicate)
            )
        )
        (optional
            (edge_centrality_map
              ,*(
                    detail::argument_with_graph_predicate<
                        detail::is_edge_property_map_of_graph
                    >
                )
              , make_shared_array_property_map(
                    num_edges(graph),
                    detail::make_default_centrality_value(
                        terminator_function
                    ),
                    detail::edge_or_dummy_property_map(graph, edge_index)
                )
            )
            (vertex_index_map
              ,*(
                    detail::argument_with_graph_predicate<
                        detail::is_vertex_to_integer_map_of_graph
                    >
                )
              , detail::vertex_or_dummy_property_map(graph, vertex_index)
            )
        )
    )
)
#else
template<typename MutableGraph, typename Done, typename EdgeCentralityMap,
         typename VertexIndexMap>
void betweenness_centrality_clustering(
    MutableGraph& graph, Done terminator_function,
    EdgeCentralityMap edge_centrality_map, VertexIndexMap vertex_index_map
)
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
{
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  typedef typename boost::remove_const<
    typename boost::remove_reference<graph_type>::type
  >::type MutableGraph;
  typedef typename boost::remove_const<
    typename boost::remove_reference<terminator_function_type>::type
  >::type Done;
  typedef typename boost::remove_const<
    typename boost::remove_reference<edge_centrality_map_type>::type
  >::type EdgeCentralityMap;
#endif
  typedef typename property_traits<EdgeCentralityMap>::value_type
    centrality_type;
  typedef typename graph_traits<MutableGraph>::edge_iterator edge_iterator;
  typedef typename graph_traits<MutableGraph>::edge_descriptor edge_descriptor;

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  if (has_no_edges(graph)) return true;

#if !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
  Done tf = terminator_function;
#endif
#else
  if (has_no_edges(graph)) return;
#endif

  // Function object that compares the centrality of edges
  indirect_cmp<EdgeCentralityMap, std::less<centrality_type> > 
    cmp(edge_centrality_map);

  bool is_done;
  do {
    brandes_betweenness_centrality(
      graph,
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
      boost::graph::keywords::_edge_centrality_map = edge_centrality_map,
      boost::graph::keywords::_vertex_index_map = vertex_index_map
#else
      boost::edge_centrality_map(edge_centrality_map)
      .vertex_index_map(vertex_index_map)
#endif
    );
    std::pair<edge_iterator, edge_iterator> edges_iters = edges(graph);
    edge_descriptor e = *max_element(edges_iters.first, edges_iters.second, cmp);
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS) && \
    !defined(BOOST_PARAMETER_HAS_PERFECT_FORWARDING)
    is_done = tf(get(edge_centrality_map, e), e, graph);
#else
    is_done = terminator_function(get(edge_centrality_map, e), e, graph);
#endif
    if (!is_done) remove_edge(e, graph);
  } while (!is_done && !has_no_edges(graph));
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  return true;
#endif
}

#if !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
/**
 * \overload
 */ 
template<typename MutableGraph, typename Done, typename EdgeCentralityMap>
void 
betweenness_centrality_clustering(MutableGraph& g, Done done,
                                  EdgeCentralityMap edge_centrality)
{
  betweenness_centrality_clustering(g, done, edge_centrality,
                                    get(vertex_index, g));
}

/**
 * \overload
 */ 
template<typename MutableGraph, typename Done>
void
betweenness_centrality_clustering(MutableGraph& g, Done done)
{
  typedef typename Done::centrality_type centrality_type;
  std::vector<centrality_type> edge_centrality(num_edges(g));
  betweenness_centrality_clustering(g, done, 
    make_iterator_property_map(edge_centrality.begin(), get(edge_index, g)),
    get(vertex_index, g));
}
#endif  // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
} // end namespace boost

#endif // BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP
