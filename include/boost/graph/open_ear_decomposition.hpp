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
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/bind.hpp>
#include <boost/graph/named_function_params.hpp>

#include <vector>
#include <algorithm>  // for std::sort


namespace boost {
  
  namespace detail {
    /**
     * TODO Some documentation about the algorithm should be here!
     *
     *
     */
    
    template <typename Vertex, typename PredMap, typename DistanceMap>
    Vertex get_lca(Vertex u, Vertex v, PredMap & pred, DistanceMap & dist) {
      if (get(dist, u) < get(dist, v)) { get_lca(u, get(pred, v), pred, dist); }
      else if (get(dist, u) > get(dist, v)) { get_lca(get(pred, u), v, pred, dist); }
      else {
        if (u != v) { get_lca(get(pred, u), get(pred, v), pred, dist); }
        else { return u; }
      }
    }
    
    template <typename Graph, typename Vertex, typename PredMap, typename DistanceMap>
    void get_distance(const Graph& g, Vertex v, PredMap & pred, DistanceMap & dist) {
      if (get(pred, v) == graph_traits<Graph>::null_vertex()) { put(dist, v, 0); }
      if (get(dist, v) == -1) {
        Vertex u = get(pred, v);
        get_distance(g, u, pred, dist);
        put(dist, v, get(dist, u) + 1);
      }
    }

    
    template <typename Graph, typename PredMap, typename DistanceMap, typename EarMap>
    void open_ear_decomposition_impl(const Graph& g, PredMap pred, DistanceMap dist, EarMap ear) {
      
      BOOST_ASSERT (num_vertices(g) >= 1); // g must also be undirected (or symmetric) and connected
      
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<Graph> ));
      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<PredMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<DistanceMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<EarMap,Edge> ));
      
      typedef typename property_traits<DistanceMap>::value_type DistanceValue;
      typedef typename property_traits<EarMap>::value_type EarValue;
      
      // For all cross-edges we need to calculate NUMBER(DIST(LCA(e))/INDEX(e))
      typedef std::pair<Edge, DistanceValue> NumberValue;
      std::vector<NumberValue> number;

      // calculate NUMBER for all tree-edges
      BGL_FORALL_EDGES_T(e, g, Graph) {
        if ((get(pred, source(e, g)) != target(e, g)) && (source(e, g) != get(pred, target(e, g)))) {
            number.push_back(std::make_pair(e, get(dist, get_lca(source(e, g), target(e, g), pred, dist))));
        } else {
          put(ear, e, 0);
        }
      }
      // sort cross-edges by number
      std::sort (number.begin(), number.end(), bind(&NumberValue::second, _1) <
          bind(&NumberValue::second, _2));
      // number ears from 1 to n
      EarValue ear_index = 1;
      for(typename std::vector<NumberValue>::iterator it = number.begin(); it != number.end(); ++it) {
        put(ear, it->first, ear_index);
        // For source and target vertex of cross-edge traverse up to lca and assign ear index
        Vertex v;
        for (int i = 0; i < 2; ++i) {
          if (i == 0) v = source(it->first, g);
          else if (i == 1) v = target(it->first, g);
          
          while (get(dist, v) != it->second) {
            Edge e = edge(v, get(pred, v), g).first;
            if (get(ear, e) == 0) put(ear, e, ear_index);
            else break;
            v = get(pred, v);
          }
          v = target(it->first, g);
        }
        ear_index++;
      }
    }
  } 
  /**
   * TODO Some documentation about the algorithm should be here!
   * 
   *
   */
  
  template <typename Graph, typename PredMap, typename DistanceMap, typename EarMap>
  void open_ear_decomposition(const Graph& g, PredMap pred, DistanceMap dist, EarMap ear) {
    // call the implementation
    detail::open_ear_decomposition_impl(g, pred, dist, ear);
  }
  
  template <typename Graph, typename PredMap, typename EarMap>
  void open_ear_decomposition(const Graph& g, PredMap pred, EarMap ear) {
    // DistanceMap needs to be calculated here
    vector_property_map<int> dist(num_vertices(g));
    BGL_FORALL_VERTICES_T(v, g, Graph) { put(dist, v, -1); }
    BGL_FORALL_VERTICES_T(v, g, Graph) { detail::get_distance(g, v, pred, dist); }
    // call the implementation
    detail::open_ear_decomposition_impl(g, pred, dist, ear);
  }
  /*
  template <typename Graph, typename P, typename T, typename R>
  void open_ear_decomposition(const Graph& g, const bgl_named_params<P, T, R>& params) {
    using namespace boost::graph::keywords;
    typedef bgl_named_params<P, T, R> params_type;
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)
    detail::open_ear_decomposition_impl(g,
                                        arg_pack[_predecessor_map],
                                        arg_pack[_distance_map | vector_property_map<int>(-1)], //TODO fill right distances in impl
                                        arg_pack[_ear_map); // TODO _ear_map is no keyword. how to implement this?
  }*/
}  //namespace boost

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_OPEN_EAR_DECOMPOSITION_HPP
