//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_MST_KRUSKAL_HPP
#define BOOST_GRAPH_MST_KRUSKAL_HPP

/*
 *Minimum Spanning Tree 
 *         Kruskal Algorithm
 *
 *Requirement:
 *      undirected graph
 */

#include <vector>
#include <queue>
#include <functional>

#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/concept/assert.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#include <boost/parameter/preprocessor.hpp>
#endif

namespace boost {

  // Kruskal's algorithm for Minimum Spanning Tree
  //
  // This is a greedy algorithm to calculate the Minimum Spanning Tree
  // for an undirected graph with weighted edges. The output will be a
  // set of edges.
  //

  namespace detail {

    template <class Graph, class OutputIterator, 
              class Rank, class Parent, class Weight>
    void
    kruskal_mst_impl(const Graph& G, 
                     OutputIterator spanning_tree_edges, 
                     Rank rank, Parent parent, Weight weight)
    {
      if (num_vertices(G) == 0) return; // Nothing to do in this case
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<Graph> ));
      BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<Graph> ));
      BOOST_CONCEPT_ASSERT(( OutputIteratorConcept<OutputIterator, Edge> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<Rank, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<Parent, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<Weight, Edge> ));
      typedef typename property_traits<Weight>::value_type W_value;
      typedef typename property_traits<Rank>::value_type R_value;
      typedef typename property_traits<Parent>::value_type P_value;
      BOOST_CONCEPT_ASSERT(( ComparableConcept<W_value> ));
      BOOST_CONCEPT_ASSERT(( ConvertibleConcept<P_value, Vertex> ));
      BOOST_CONCEPT_ASSERT(( IntegerConcept<R_value> ));

      disjoint_sets<Rank, Parent>  dset(rank, parent);

      typename graph_traits<Graph>::vertex_iterator ui, uiend;
      for (boost::tie(ui, uiend) = vertices(G); ui != uiend; ++ui)
        dset.make_set(*ui);

      typedef indirect_cmp<Weight, std::greater<W_value> > weight_greater;
      weight_greater wl(weight);
      std::priority_queue<Edge, std::vector<Edge>, weight_greater> Q(wl);
      /*push all edge into Q*/
      typename graph_traits<Graph>::edge_iterator ei, eiend;
      for (boost::tie(ei, eiend) = edges(G); ei != eiend; ++ei) 
        Q.push(*ei);

      while (! Q.empty()) {
        Edge e = Q.top();
        Q.pop();
        Vertex u = dset.find_set(source(e, G));
        Vertex v = dset.find_set(target(e, G));
        if ( u != v ) {
          *spanning_tree_edges++ = e;
          dset.link(u, v);
        }
      }
    }

  } // namespace detail 

  // Named Parameters Variants

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), kruskal_minimum_spanning_tree, ::boost::graph::keywords::tag,
    (required
      (graph, *(detail::argument_predicate<is_vertex_list_graph>))
    )
    (deduced
      (required
        (result, *(detail::argument_predicate<detail::is_iterator>))
      )
      (optional
        (weight_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_edge_property_map_of_graph
            >
          )
          ,detail::edge_or_dummy_property_map(graph, edge_weight)
        )
        (vertex_index_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_to_integer_map_of_graph
            >
          )
          ,detail::vertex_or_dummy_property_map(graph, vertex_index)
        )
        (predecessor_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_to_vertex_map_of_graph
            >
          )
          ,make_shared_array_property_map(
            num_vertices(graph),
            detail::get_null_vertex(graph),
            vertex_index_map
          )
        )
      )
    )
    (optional
      (rank_map
        ,*(
          detail::argument_with_graph_predicate<
            detail::is_vertex_to_integer_map_of_graph
          >
        )
        ,make_shared_array_property_map(
          num_vertices(graph),
          num_vertices(graph) - num_vertices(graph),
          vertex_index_map
        )
      )
    )
  )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), kruskal_minimum_spanning_tree, ::boost::graph::keywords::tag,
    (required
      (graph, *)
      (result, *)
    )
    (optional
      (weight_map
        ,*
        ,detail::edge_or_dummy_property_map(graph, edge_weight)
      )
      (vertex_index_map
        ,*
        ,detail::vertex_or_dummy_property_map(graph, vertex_index)
      )
      (predecessor_map
        ,*
        ,make_shared_array_property_map(
          num_vertices(graph),
          detail::get_null_vertex(graph),
          vertex_index_map
        )
      )
      (rank_map
        ,*
        ,make_shared_array_property_map(
          num_vertices(graph),
          num_vertices(graph) - num_vertices(graph),
          vertex_index_map
        )
      )
    )
  )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
  {
    if (0 < num_vertices(graph))
      detail::kruskal_mst_impl
        (graph, result, rank_map, predecessor_map, weight_map);
    return true;
  }
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <class Graph, class OutputIterator>
  inline void 
  kruskal_minimum_spanning_tree(const Graph& g,
                                OutputIterator spanning_tree_edges)
  {
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;
    if (num_vertices(g) == 0) return; // Nothing to do in this case
    typename graph_traits<Graph>::vertices_size_type
      n = num_vertices(g);
    std::vector<size_type> rank_map(n);
    std::vector<vertex_t> pred_map(n);

    detail::kruskal_mst_impl
      (g, spanning_tree_edges, 
       make_iterator_property_map(rank_map.begin(), get(vertex_index, g), rank_map[0]),
       make_iterator_property_map(pred_map.begin(), get(vertex_index, g), pred_map[0]),
       get(edge_weight, g));
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

  template <class Graph, class OutputIterator, class P, class T, class R>
  inline void
  kruskal_minimum_spanning_tree(const Graph& g,
                                OutputIterator spanning_tree_edges, 
                                const bgl_named_params<P, T, R>& params)
  {
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;
    if (num_vertices(g) == 0) return; // Nothing to do in this case
    typename graph_traits<Graph>::vertices_size_type n;
    n = is_default_param(get_param(params, vertex_rank))
                                   ? num_vertices(g) : 1;
    std::vector<size_type> rank_map(n);
    n = is_default_param(get_param(params, vertex_predecessor))
                                   ? num_vertices(g) : 1;
    std::vector<vertex_t> pred_map(n);
    
    detail::kruskal_mst_impl
      (g, spanning_tree_edges, 
       choose_param
       (get_param(params, vertex_rank), 
        make_iterator_property_map
        (rank_map.begin(), 
         choose_pmap(get_param(params, vertex_index), g, vertex_index), rank_map[0])),
       choose_param
       (get_param(params, vertex_predecessor), 
        make_iterator_property_map
        (pred_map.begin(), 
         choose_const_pmap(get_param(params, vertex_index), g, vertex_index), 
         pred_map[0])),
       choose_const_pmap(get_param(params, edge_weight), g, edge_weight));
  }
    
} // namespace boost


#endif // BOOST_GRAPH_MST_KRUSKAL_HPP

