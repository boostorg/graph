#ifndef BOOST_GRAPH_DAG_SHORTEST_PATHS_HPP
#define BOOST_GRAPH_DAG_SHORTEST_PATHS_HPP

#include <boost/graph/relax.hpp>
#include <boost/graph/depth_first_search.hpp>

// UNDER CONSTRUCTION

// single-source shortest paths for a Directed Acyclic Graph (DAG)

namespace boost {

  namespace detail {

    template <class DijkstraVisitor, 
      class WeightMap, class DistanceMap, 
      class BinaryFunction, class BinaryPredicate>
    struct dag_sp_visitor : public dfs_visitor<>
    {
      dag_sp_visitor(DijkstraVisitor vis, WeightMap w, DistanceMap d, 
                     BinaryFunction cmb, BinaryPredicate cmp)
        : m_vis(vis), m_weight(w), m_distance(d), 
          m_combine(cmb), m_compare(cmp) { }

      template <class Vertex, class Graph> 
      void finish_vertex(const Vertex& u, const Graph& g)
      {
        m_vis.examine_vertex(u, g);
        typename graph_traits<Graph>::out_edge_iterator e, e_end;
        for (tie(e, e_end) = out_edges(u, g); e != e_end; ++e) {
          m_vis.discover_vertex(target(*e, g), g);
          bool decreased = relax(*e, g, m_weight, m_distance, 
                                 m_combine, m_compare);
          if (decreased)
            m_vis.edge_relaxed(*e, g);
          else
            m_vis.edge_not_relaxed(*e, g);
        }
        m_vis.finish_vertex(u, g);
      }

      WeightMap m_weight;
      DistanceMap m_distance;
      BinaryFunction m_combine;
      BinaryPredicate m_compare;
    };
    
    // Initalize distances and call depth first search
    template <class VertexListGraph, class DijkstraVisitor, 
              class DistanceMap, class WeightMap, class ColorMap, 
              class Compare, class Combine, 
              class DistInf, class DistZero>
    inline void
    dag_sp_dispatch3
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color,
       DijkstraVisitor vis, Compare compare, Combine combine, 
       DistInf init, DistZero zero)
    {
      typename graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
      for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
        put(distance, *ui, init());
        vis.initialize_vertex(*ui, g);
      }

      put(distance, s, zero());
      vis.discover_vertex(s, g);

      dag_sp_visitor<DijkstraVisitor, WeightMap, DistanceMap,
        Combine, Compare> v(vis, weight, distance, combine, compare);

      depth_first_visit(g, s, v, color);
    }

    // Defaults are the same as Dijkstra's algorithm

    // Handle Distance Compare, Combine, Inf and Zero defaults
    template <class VertexListGraph, class DijkstraVisitor, 
              class DistanceMap, class WeightMap, class ColorMap,
              class Params>
    inline void
    dag_sp_dispatch2
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
       DijkstraVisitor vis, const Params& params)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      detail::dag_sp_dispatch3
        (g, s, distance, weight, color, vis, 
         choose_param(get_param(params, distance_compare_t()), std::less<D>()),
         choose_param(get_param(params, distance_combine_t()), std::plus<D>()),
         choose_param(get_param(params, distance_inf_t()), 
                      generate_infinity<D>()),
         choose_param(get_param(params, distance_zero_t()), 
                      generate_zero<D>()));
    }

    // Handle DistanceMap and ColorMap defaults
    template <class VertexListGraph, class DijkstraVisitor, 
              class DistanceMap, class WeightMap, class ColorMap,
              class IndexMap, class Params>
    inline void
    dag_sp_dispatch1
      (const VertexListGraph& g,
       typename graph_traits<VertexListGraph>::vertex_descriptor s, 
       DistanceMap distance, WeightMap weight, ColorMap color, IndexMap id,
       DijkstraVisitor vis, const Params& params)
    {
      typedef typename property_traits<WeightMap>::value_type T;
      typename std::vector<T>::size_type n;
      n = is_default_param(distance) ? num_vertices(g) : 1;
      std::vector<T> distance_map(n);
      n = is_default_param(color) ? num_vertices(g) : 1;
      std::vector<default_color_type> color_map(n);
      dag_sp_dispatch2
        (g, s, 
         choose_param(distance, 
                      make_iterator_property_map(distance_map.begin(), id,
						 distance_map[0])),
         weight, 
         choose_param(color,
                      make_iterator_property_map(color_map.begin(), id, color_map[0])),
         vis, params);
    }
    
  } // namespace detail 
  
  template <class VertexListGraph, class Param, class Tag, class Rest>
  inline void
  dag_shortest_paths
    (const VertexListGraph& g,
     typename graph_traits<VertexListGraph>::vertex_descriptor s,
     const bgl_named_params<Param,Tag,Rest>& params)
  {
    // assert that the graph is directed...
    null_visitor null_vis;
    detail::dag_sp_dispatch1
      (g, s, 
       get_param(params, vertex_distance),
       choose_pmap(get_param(params, edge_weight), g, edge_weight),
       get_param(params, vertex_color),
       choose_pmap(get_param(params, vertex_index), g, vertex_index),
       choose_param(get_param(params, graph_visitor),
                    make_dijkstra_visitor(null_vis)),
       params);
  }
  
} // namespace boost

#endif // BOOST_GRAPH_DAG_SHORTEST_PATHS_HPP
