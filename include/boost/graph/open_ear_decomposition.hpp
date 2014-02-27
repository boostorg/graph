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
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <vector>
#include <algorithm>

#include <boost/concept_check.hpp>
#include <boost/graph/buffer_concepts.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/graph_concepts.hpp>

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
      
      // For all cross-edges we need to calculate NUMBER(DIST(LCA(e))/INDEX(e))
      typedef std::vector<std::pair<Edge, DistanceValue>> NumberVector;
      NumberVector number;

      // calculate NUMBER for all tree-edges
      BGL_FORALL_EDGES_T(e, g, Graph) {
        if ((get(pred, source(e, g)) != target(e, g)) && (source(e, g) != get(pred, target(e, g))) {
            number.push_back(std::make_pair(e, get(dist, get_lca(source(e, g), target(e, g), pred, dist))));
        } else {
          put(ear, e, 0);
        }
      }
      // sort cross-edges by number
      std::sort (number.begin(), number.end(), lexicographical_sort);
      // number ears from 1 to n
      EarValue ear_index = 1;
      for(NumberVector::iterator it = number.begin(); it != number.end(); ++it) {
        put(ear, it->first, ear_index);
        // For source and target vertex of cross-edge traverse up to lca and assign ear index
        for (int i = 0; i < 2; ++i) {
          if (i == 0) Vertex v = source(it->first, g);
          else if (i == 1) Vertex v = target(it->first, g);
          
          while (get(dist, v) != it->second) {
            std::tie(Edge e, bool exists) = edge(v, get(pred, v), g);
            if (exists) {
              if (get(ear, e) == 0) put(ear, e, ear_index);
              else break;
            }
            v = get(pred, v);
          }
          v = target(it->first, g);
        }
        ear_index++;
      }
    }
    
    template <typename A, typename B>
    bool lexicographical_sort(A a, B b) {
      if (a.second == b.second) return (a.first > b.first);
      else return (a.second > b.second);
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
        put(dist, v, get(dist, get(pred, v)) + 1);
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
