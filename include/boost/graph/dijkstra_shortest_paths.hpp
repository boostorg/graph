//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
//
//
// Revision History:
//   04 April 2001: Added named parameter variant. (Jeremy Siek)
//   01 April 2001: Modified to use new <boost/limits.hpp> header. (JMaddock)
//
#ifndef BOOST_GRAPH_DIJKSTRA_HPP
#define BOOST_GRAPH_DIJKSTRA_HPP

#include <functional>
#include <boost/limits.hpp>
#include <boost/graph/uniform_cost_search.hpp>
#include <boost/graph/named_function_params.hpp>

namespace boost {

#ifndef BOOST_GRAPH_NO_DEPRECATED

  // Variant (1)
  template <class VertexListGraph>
  inline void
  dijkstra_shortest_paths
    (VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s)
  {
    dijkstra_shortest_paths(g, s, get(vertex_distance, g));
  }

  // Variant (2)
  template <class VertexListGraph, class DistanceMap>
  inline void
  dijkstra_shortest_paths
    (const VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s, 
     DistanceMap d)
  {
    null_visitor null_vis;
    dijkstra_shortest_paths(g, s, d,
                            get(edge_weight, g), 
                            get(vertex_color, g), 
                            get(vertex_index, g), 
                            make_ucs_visitor(null_vis));
  }

  // Variant (3)
  template <class VertexListGraph, class DistanceMap, class UniformCostVisitor>
  inline void
  dijkstra_shortest_paths
    (const VertexListGraph& g, 
     typename graph_traits<VertexListGraph>::vertex_descriptor s, 
     DistanceMap d, UniformCostVisitor visit)
  {
    dijkstra_shortest_paths(g, s, d, 
                            get(edge_weight, g),
                            get(vertex_color, g),
                            get(vertex_index, g),
                            visit);
  }

  // Variant (4)
  template <class VertexListGraph, class UniformCostVisitor, 
            class DistanceMap, class WeightMap, class ColorMap, class IndexMap>
  inline void
  dijkstra_shortest_paths
    (const VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s, 
     DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
     UniformCostVisitor vis)
  {
    typedef typename property_traits<DistanceMap>::value_type D;
    typedef typename property_traits<WeightMap>::value_type W;
    std::less<D> compare;
    std::plus<W> combine;

    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
      put(distance, *ui, std::numeric_limits<D>::max());

    put(distance, s, D());
    uniform_cost_search(g, s, distance, weight, color, id, 
                        compare, combine, vis);
  }

#endif // BOOST_GRAPH_NO_DEPRECATED

  namespace detail {

    template <typename D>
    struct generate_infinity {
      D operator()() const { return std::numeric_limits<D>::max(); }
    };
    template <typename D>
    struct generate_zero {
      D operator()() const { return D(); }
    };

    // Initalize distances and call uniform cost search
    template <class VertexListGraph, class UniformCostVisitor, 
              class DistanceMap, class WeightMap, class ColorMap, 
              class IndexMap, class Compare, class Combine, 
              class DistInf, class DistZero>
    inline void
    dijkstra_dispatch3
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
       UniformCostVisitor vis, Compare compare, Combine combine, 
       DistInf init, DistZero zero)
    {
      typename boost::graph_traits<VertexListGraph>::vertex_iterator
        ui, ui_end;
      for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
        put(distance, *ui, init());

      put(distance, s, zero());
      uniform_cost_search(g, s, distance, weight, color, id,
                          compare, combine, vis);
    }

    // Handle Distance Compare, Combine, Inf and Zero defaults
    template <class VertexListGraph, class UniformCostVisitor, 
              class DistanceMap, class WeightMap, class ColorMap,
              class IndexMap, class Params>
    inline void
    dijkstra_dispatch2
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
       UniformCostVisitor vis, const Params& params)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      detail::dijkstra_dispatch3
        (g, s, distance, weight, color, id, vis, 
         choose_param(get_param(params, distance_compare_t()), std::less<D>()),
         choose_param(get_param(params, distance_combine_t()), std::plus<D>()),
         choose_param(get_param(params, distance_inf_t()), 
                      generate_infinity<D>()),
         choose_param(get_param(params, distance_zero_t()), 
                      generate_zero<D>()));
    }

    // Handle DistanceMap and ColorMap defaults
    template <class VertexListGraph, class UniformCostVisitor, 
              class DistanceMap, class WeightMap, class ColorMap,
              class IndexMap, class Params>
    inline void
    dijkstra_dispatch1
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
       UniformCostVisitor vis, const Params& params)
    {
      typedef typename property_traits<WeightMap>::value_type T;
      typename std::vector<T>::size_type n;
      n = is_default_param(distance) ? num_vertices(g) : 0;
      std::vector<T> distance_map(n);
      n = is_default_param(color) ? num_vertices(g) : 0;
      std::vector<default_color_type> color_map(n);
      dijkstra_dispatch2
        (g, s, 
         choose_param(distance, 
                      make_iterator_property_map(distance_map.begin(), id)),
         weight, 
         choose_param(color,
                      make_iterator_property_map(color_map.begin(), id)),
         id, vis, params);
    }
  } // namespace detail

  // Named Parameter Variant
  template <class VertexListGraph, class Param, class Tag, class Rest>
  inline void
  dijkstra_shortest_paths
    (const VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s,
     const bgl_named_params<Param,Tag,Rest>& params)
  {
    // Default for edge weight and vertex index map is to ask for them
    // from the graph.  Default for the visitor is null_visitor.
    null_visitor null_vis;
    detail::dijkstra_dispatch1
      (g, s, 
       get_param(params, vertex_distance),
       choose_pmap(get_param(params, edge_weight), g, edge_weight),
       get_param(params, vertex_color),
       choose_pmap(get_param(params, vertex_index), g, vertex_index),
       choose_param(get_param(params, graph_visitor),
                    make_ucs_visitor(null_vis)),
       params);
  }

} // namespace boost

#endif /* BOOST_GRAPH_DIJKSTRA_HPP*/
