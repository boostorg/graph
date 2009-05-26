//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Copyright 2009 Trustees of Indiana University.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek, Michael Hansen
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_DIJKSTRA_NO_COLOR_MAP_HPP
#define BOOST_GRAPH_DIJKSTRA_NO_COLOR_MAP_HPP

#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/relax.hpp>
#include <boost/pending/relaxed_heap.hpp>
#include <boost/graph/detail/d_ary_heap.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/iteration_macros.hpp>

namespace boost {

template <typename Graph, typename DijkstraVisitor,
          typename PredecessorMap, typename DistanceMap,
          typename WeightMap, typename VertexIndexMap,
          typename DistanceCompare, typename DistanceWeightCombine,
          typename DistanceInfinity, typename DistanceZero>
void dijkstra_shortest_paths_no_color_map
  (const Graph& graph,
   typename graph_traits<Graph>::vertex_descriptor start_vertex,
   PredecessorMap predecessor_map,
   DistanceMap distance_map,
   WeightMap weight_map,
   VertexIndexMap vertex_index_map,
   DistanceCompare distance_compare,
   DistanceWeightCombine distance_weight_combine,
   DistanceInfinity distance_infinity,
   DistanceZero distance_zero,
   DijkstraVisitor visitor)
{
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  typedef typename property_traits<DistanceMap>::value_type Distance;
  typedef typename property_traits<WeightMap>::value_type Weight;
  
  typedef indirect_cmp<DistanceMap, DistanceCompare> DistanceIndirectCompare;
  DistanceIndirectCompare
    distance_indirect_compare(distance_map, distance_compare);

  // Choose vertex queue type
#if BOOST_GRAPH_DIJKSTRA_USE_RELAXED_HEAP
  typedef relaxed_heap<Vertex, DistanceIndirectCompare, VertexIndexMap>
    VertexQueue;
  VertexQueue vertex_queue(num_vertices(graph),
                           distance_indirect_compare,
                           vertex_index_map);
#else
  // Default - use d-ary heap (d = 4)
  typedef
    detail::vertex_property_map_generator<Graph, VertexIndexMap, std::size_t>
    IndexInHeapMapHelper;
  typedef typename IndexInHeapMapHelper::type IndexInHeapMap;
  typedef
    d_ary_heap_indirect<Vertex, 4, IndexInHeapMap, DistanceMap, DistanceCompare>
    VertexQueue;

  boost::scoped_array<std::size_t> index_in_heap_map_holder;
  IndexInHeapMap index_in_heap =
    IndexInHeapMapHelper::build(graph, vertex_index_map,
                                index_in_heap_map_holder);  
  VertexQueue vertex_queue(distance_map, index_in_heap, distance_compare);
#endif

  // Initialize vertices
  BGL_FORALL_VERTICES_T(current_vertex, graph, Graph) {
    visitor.initialize_vertex(current_vertex, graph);
    
    // Default all distances to infinity
    put(distance_map, current_vertex, distance_infinity);

    // Default all vertex predecessors to the vertex itself
    put(predecessor_map, current_vertex, current_vertex);
  }

  // Set distance for start_vertex to zero
  put(distance_map, start_vertex, distance_zero);

  // Add vertex to the queue
  vertex_queue.push(start_vertex);

  // Starting vertex will always be the first discovered vertex
  visitor.discover_vertex(start_vertex, graph);

  while (!vertex_queue.empty()) {
    Vertex min_vertex = vertex_queue.top();
    vertex_queue.pop();
    
    visitor.examine_vertex(min_vertex, graph);

    // Check if any other vertices can be reached
    Distance min_vertex_distance = get(distance_map, min_vertex);
    
    if (!distance_compare(min_vertex_distance, distance_infinity)) {
      // This is the minimum vertex, so all other vertices are unreachable
      return;
    }

    // Examine neighbors of min_vertex
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    typename graph_traits<Graph>::out_edge_iterator edge_iter, edge_iter_end;
    BGL_FORALL_OUTEDGES_T(min_vertex, current_edge, graph, Graph) {
      visitor.examine_edge(current_edge, graph);
      
      // Check if the edge has a negative weight
      if (distance_compare(get(weight_map, current_edge), distance_zero)) {
        boost::throw_exception(negative_edge());
      }
      
      Vertex neighbor_vertex = target(current_edge, graph);
      Distance neighbor_vertex_distance = get(distance_map, neighbor_vertex);
      bool is_neighbor_undiscovered = 
        !distance_compare(neighbor_vertex_distance, distance_infinity);

      // Attempt to relax the edge
      bool was_edge_relaxed = relax(current_edge, graph, weight_map,
        predecessor_map, distance_map,
        distance_weight_combine, distance_compare);

      if (was_edge_relaxed) {
        vertex_queue.update(neighbor_vertex);
        visitor.edge_relaxed(current_edge, graph);
      } else {
        visitor.edge_not_relaxed(current_edge, graph);
      }

      if (is_neighbor_undiscovered) {
        visitor.discover_vertex(neighbor_vertex, graph);
        vertex_queue.push(neighbor_vertex);
      }
    } // end out edge iteration

    visitor.finish_vertex(min_vertex, graph);
  } // end while queue not empty
}

} // namespace boost

#endif // BOOST_GRAPH_DIJKSTRA_NO_COLOR_MAP_HPP
