// Copyright (c) 2014 Stefan Hammer, University of Vienna
// Copyright (c) 2014 Jakob Lykke Andersen, University of Southern Denmark
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_OPEN_EAR_DECOMPOSITION_HPP
#define BOOST_GRAPH_OPEN_EAR_DECOMPOSITION_HPP

#include <boost/concept/assert.hpp>
#include <boost/assert.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/iteration_macros.hpp>

#include <vector>

#include <boost/concept_check.hpp>
#include <boost/graph/buffer_concepts.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <boost/graph/named_function_params.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/config.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/overloading.hpp>
#include <boost/static_assert.hpp>



namespace boost {
  
  namespace detail {
    /**
     * TODO Some documentation about the algorithm should be here!
     *
     *
     */
    
    template <typename Graph, typename PredMap, typename DistanceMap, typename EarMap>
    void open_ear_decomposition_impl(const Graph& g, PredMap pred, DistanceMap dist, EarMap ear) {
      
      BOOST_CONCEPT_ASSERT(( GraphConcept<Graph> ));
      BOOST_ASSERT (num_vertices(g) >= 1); // g must also be undirected (or symmetric) and connected
      
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<PredMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<DistanceMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<EarMap,Edge> ));
      
      typedef typename property_traits<PredMap>::value_type PredValue;
      typedef typename property_traits<DistanceMap>::value_type DistanceValue;
      typedef typename property_traits<EarMap>::value_type EarValue;
      
      typename graph_traits<Graph>::edge_iterator ei, ei_end;
      
      // Needed container are: 
      struct {
        ei;
        DistanceValue;
      }
      typedef std::vector<> Number;
      
      
    }
    
    template <typename Graph, typename PredMap, typename DistanceMap>
    typename graph_traits<Graph>::vertex_descriptor Vertex;
    Vertex get_lca(Vertex u, Vertex v, PredMap& pred, DistanceMap& dist) {
      if (get(dist, u) < get(dist, v)) { get_lca(u, get(pred, v), pred, dist); }
      else if (get(dist, u) > get(dist, v)) { get_lca(get(pred, u), v, pred, dist); }
      else {
        if (u != v) { get_lca(get(pred, u), get(pred, v), pred, dist); }
        else { return u; }
      }
    }
    
    template <typename Graph, typename PredMap, typename DistanceMap>
    void get_distance(typename graph_traits<Graph>::vertex_descriptor v, PredMap& pred, DistanceMap& dist) {
      if (get(pred, v) == graph_traits<Graph>::null_vertex()) { put(dist, v, 0); }
      if (get(dist, v) == -1) {
        get_distance(get(pred, v), pred, dist);
        put(dist, v) = get(dist, get(pred, v)) + 1;
      }
    }
  
  /**
   * TODO Some documentation about the algorithm should be here!
   * 
   *
   */
  
  template <typename Graph, typename PredMap, typename EarMap>
  void open_ear_decomposition(const Graph& g, PredMap pred, EarMap ear) {
    // DistanceMap needs to be calculated here
    typedef std::vector<int> DistanceMap;
    BGL_FORALL_VERTICES_T(v, g, Graph) put(dist, v, -1);
    BGL_FORALL_VERTICES_T(v, g, Graph) detail::get_distance(v, pred, dist);
    
    detail::open_ear_decomposition_impl(g, pred, dist, ear);
  }
  
  template <typename Graph, typename PredMap, typename DistanceMap, typename EarMap>
  void open_ear_decomposition(const Graph& g, PredMap pred, DistanceMap dist, EarMap ear) {
    // call the implementation
    detail::open_ear_decomposition_impl(g, pred, dist, ear);
  }

}

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_OPEN_EAR_DECOMPOSITION_HPP
