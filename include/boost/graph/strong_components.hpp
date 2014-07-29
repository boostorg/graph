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

#ifndef BOOST_GRAPH_STRONG_COMPONENTS_HPP
#define BOOST_GRAPH_STRONG_COMPONENTS_HPP

#include <stack>
#include <boost/config.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/type_traits/conversion_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/graph/overloading.hpp>
#include <boost/concept/assert.hpp>

namespace boost {

  //==========================================================================
  // This is Tarjan's algorithm for strongly connected components
  // from his paper "Depth first search and linear graph algorithms".
  // It calculates the components in a single application of DFS.
  // We implement the algorithm as a dfs-visitor.
  template <typename visitor_t, typename graph_t, typename time_t>
  class SccVisitorConcept {
  public:
    void constraints() {
      BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<visitor_t,graph_t> ));
      vis.discover_component(v, c, g);
      vis.add_to_component(v, v, c, g);
      vis.finish_component(v, c, g);
    }
  private:
    visitor_t vis;
    graph_t g;
    time_t c;
    typename graph_traits<graph_t>::vertex_descriptor v;
  };
   
  // A new named parameter for the strong components visitor.
  // What's the right way to do that?
  struct strong_components_visitor_t {}; 

  namespace detail {
    template<typename graph_t>
    struct NullSccVisitor
      : public dfs_visitor<>  {
      typedef typename graph_traits<graph_t>::vertex_descriptor vertex_descriptor;

      template <typename time_t>
      void discover_component(vertex_descriptor, time_t, const graph_t&) {
      }
     
      template <typename time_t>
      void add_to_component(vertex_descriptor, vertex_descriptor, time_t, const graph_t&) {
      }
     
      template <typename time_t>
      void finish_component(vertex_descriptor, time_t, const graph_t&) {
      }
    };
    
    template <typename ComponentMap, typename RootMap, typename DiscoverTime, 
	      typename SccVisitor,
              typename Stack>
    class tarjan_scc_visitor
    {
      typedef typename property_traits<ComponentMap>::value_type comp_type;
      typedef typename property_traits<DiscoverTime>::value_type time_type;
    public:
      tarjan_scc_visitor(ComponentMap comp_map, RootMap r, DiscoverTime d, 
                         comp_type& c_, SccVisitor &scc_vis_, Stack& s_)
        : c(c_), comp(comp_map), root(r), discover_time(d),
	  dfs_time(time_type()), s(s_), scc_vis(scc_vis_)
      {
      }

      template <typename Graph>
      void discover_vertex(typename graph_traits<Graph>::vertex_descriptor v,
                           const Graph& g) {
        put(root, v, v);
        put(comp, v, (std::numeric_limits<comp_type>::max)());
        put(discover_time, v, dfs_time++);
        s.push(v);
	// Forward the call to the SccVisitor.
	scc_vis.discover_vertex(v, g);
      }
      template <typename Graph>
      void finish_vertex(typename graph_traits<Graph>::vertex_descriptor v,
                         const Graph& g) {
        typename graph_traits<Graph>::vertex_descriptor w;
        typename graph_traits<Graph>::out_edge_iterator ei, ei_end;
        for (boost::tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
          w = target(*ei, g);
          if (get(comp, w) == (std::numeric_limits<comp_type>::max)())
            put(root, v, this->min_discover_time(get(root,v), get(root,w)));
        }
        if (get(root, v) == v) {
	  scc_vis.discover_component(v, c, g);
	  do {
            w = s.top(); s.pop();
            put(comp, w, c);
	    put(root, w, v);
	    scc_vis.add_to_component(w, v, c, g);
          } while (w != v);
	  scc_vis.finish_component(v, c, g);
          ++c;
        }
	// Forward the call to the SccVisitor.
	scc_vis.finish_vertex(v, g);
      }

      template <class Graph>
      void initialize_vertex(typename graph_traits<Graph>::vertex_descriptor u, const Graph& g) {
	scc_vis.initialize_vertex(u, g);
      }
      template <class Graph>
      void start_vertex(typename graph_traits<Graph>::vertex_descriptor u, const Graph& g) {
	scc_vis.start_vertex(u, g);
      }
      template <class Graph>
      void examine_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.examine_edge(u, g);
      }
      template <class Graph>
      void tree_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.tree_edge(u, g);
      }
      template <class Graph>
      void back_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.back_edge(u, g);
      }
      template <class Graph>
      void forward_or_cross_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.forward_or_cross_edge(u, g);
      } 
      template <class Graph>
      void finish_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.finish_edge(u, g);
      }
      template <class Graph>
      void finish_tree_edge(typename graph_traits<Graph>::edge_descriptor u, const Graph& g) {
	scc_vis.finish_tree_edge(u, g);
      }

    private:
      template <typename Vertex>
      Vertex min_discover_time(Vertex u, Vertex v) {
        return get(discover_time, u) < get(discover_time,v) ? u : v;
      }

      comp_type& c;
      ComponentMap comp;
      RootMap root;
      DiscoverTime discover_time;
      time_type dfs_time;
      Stack& s;
      SccVisitor &scc_vis;
    };
    
    template <class Graph, class ComponentMap, class SccVisitor, class RootMap,
              class DiscoverTime, class P, class T, class R>
    typename property_traits<ComponentMap>::value_type
    strong_components_impl
      (const Graph& g,    // Input
       ComponentMap comp, // Output
       SccVisitor scc_vis, // Input
       // Internal record keeping
       RootMap root, 
       DiscoverTime discover_time,
       const bgl_named_params<P, T, R>& params)
    {
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<ComponentMap, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<RootMap, Vertex> ));
      typedef typename property_traits<RootMap>::value_type RootV;
      BOOST_CONCEPT_ASSERT(( ConvertibleConcept<RootV, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<DiscoverTime, Vertex> ));
      typedef typename property_traits<DiscoverTime>::value_type Time;
      BOOST_CONCEPT_ASSERT(( SccVisitorConcept<SccVisitor, Graph, Time> ));
      
      typename property_traits<ComponentMap>::value_type total = 0;

      std::stack<Vertex> s;
      detail::tarjan_scc_visitor<ComponentMap, RootMap, DiscoverTime, SccVisitor,
				 std::stack<Vertex> > vis(comp, root, discover_time,
							  total, scc_vis, s);
      depth_first_search(g, params.visitor(vis));
      return total;
    }

    //-------------------------------------------------------------------------
    // The dispatch functions handle the defaults for the rank and discover
    // time property maps.
    // dispatch with class specialization to avoid VC++ bug
    
    // Level 2 dispatch: get time_map
    template <class DiscoverTimeMap>
    struct strong_comp_dispatch2 {
      template <class Graph, class ComponentMap, class SccVisitor, class RootMap, class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
	    SccVisitor scc_vis,
            RootMap r_map,
            const bgl_named_params<P, T, R>& params,
            DiscoverTimeMap time_map)
      {
        return strong_components_impl(g, comp, scc_vis, r_map, time_map, params);
      }
    };
    
    template <>
    struct strong_comp_dispatch2<param_not_found> {
      template <class Graph, class ComponentMap, class SccVisitor, class RootMap,
                class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
	    SccVisitor scc_vis,
            RootMap r_map,
            const bgl_named_params<P, T, R>& params,
            param_not_found)
      {
        typedef typename graph_traits<Graph>::vertices_size_type size_type;
        size_type       n = num_vertices(g) > 0 ? num_vertices(g) : 1;
        std::vector<size_type> time_vec(n);
        return strong_components_impl(g, comp, scc_vis, r_map,
				      make_iterator_property_map(time_vec.begin(), choose_const_pmap
								 (get_param(params, vertex_index),
								  g, vertex_index), time_vec[0]),
				      params);
      }
    };

    template <class Graph, class ComponentMap, class SccVisitor, class RootMap,
              class P, class T, class R, class DiscoverTimeMap>
    inline typename property_traits<ComponentMap>::value_type
    scc_helper2(const Graph& g,
                ComponentMap comp,
		SccVisitor scc_vis,
                RootMap r_map,
                const bgl_named_params<P, T, R>& params,
                DiscoverTimeMap time_map)
    {
      return strong_comp_dispatch2<DiscoverTimeMap>::apply(g, comp, scc_vis, r_map, params, time_map);
    }

    // Level 1 dispatch: get r_map
    template <class RootMap>
    struct strong_comp_dispatch1 {

      template <class Graph, class ComponentMap, class SccVisitor, class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
	    SccVisitor scc_vis,
            const bgl_named_params<P, T, R>& params,
            RootMap r_map)
      {
        return scc_helper2(g, comp, scc_vis, r_map, params, get_param(params, vertex_discover_time));
      }
    };
    
    template <>
    struct strong_comp_dispatch1<param_not_found> {

      template <class Graph, class ComponentMap, class SccVisitor, 
                class P, class T, class R>
      inline static typename property_traits<ComponentMap>::value_type
      apply(const Graph& g,
            ComponentMap comp,
	    SccVisitor scc_vis,
            const bgl_named_params<P, T, R>& params,
            param_not_found)
      {
        typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
        typename std::vector<Vertex>::size_type
          n = num_vertices(g) > 0 ? num_vertices(g) : 1;
        std::vector<Vertex> root_vec(n);
        return scc_helper2(g, comp, scc_vis,
			   make_iterator_property_map(root_vec.begin(), choose_const_pmap
						      (get_param(params, vertex_index),
						       g, vertex_index), root_vec[0]),
			   params, 
			   get_param(params, vertex_discover_time));
      }
    };

    template <class Graph, class ComponentMap, class SccVisitor, class RootMap,
              class P, class T, class R>
    inline typename property_traits<ComponentMap>::value_type
    scc_helper1(const Graph& g,
		ComponentMap comp,
		SccVisitor scc_vis,
		const bgl_named_params<P, T, R>& params,
		RootMap r_map)
    {
      return detail::strong_comp_dispatch1<RootMap>::apply(g, comp, scc_vis, 
							   params, r_map);
    }

  } // namespace detail 

  template <class Graph, class ComponentMap, class SccVisitor,
            class P, class T, class R>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp, SccVisitor scc_vis,
                    const bgl_named_params<P, T, R>& params
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    typedef typename graph_traits<Graph>::directed_category DirCat;
    BOOST_STATIC_ASSERT((is_convertible<DirCat*, directed_tag*>::value == true));
    return detail::scc_helper1(g, comp, scc_vis, params, 
                               get_param(params, vertex_root_t()));
  }

  template <class Graph, class ComponentMap,
            class P, class T, class R>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp,
                    const bgl_named_params<P, T, R>& params
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    return strong_components(g, comp, detail::NullSccVisitor<Graph>(), params);
  }

  template <class Graph, class ComponentMap, class SccVisitor>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp, SccVisitor scc_vis
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    typedef typename graph_traits<Graph>::directed_category DirCat;
    BOOST_STATIC_ASSERT((is_convertible<DirCat*, directed_tag*>::value == true));
    bgl_named_params<int, int> params(0);
    return strong_components(g, comp, scc_vis, params);
  }

  template <class Graph, class ComponentMap>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g, ComponentMap comp
                    BOOST_GRAPH_ENABLE_IF_MODELS_PARM(Graph, vertex_list_graph_tag))
  {
    return strong_components(g, comp, detail::NullSccVisitor<Graph>());
  }
  
  template <typename Graph, typename ComponentMap, typename ComponentLists>
  void build_component_lists
    (const Graph& g,
     typename graph_traits<Graph>::vertices_size_type num_scc,
     ComponentMap component_number,
     ComponentLists& components)
  {
    components.resize(num_scc);
    typename graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
      components[component_number[*vi]].push_back(*vi);
  }


} // namespace boost

#include <queue>
#include <vector>
#include <boost/graph/transpose_graph.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/connected_components.hpp> // for components_recorder

namespace boost {

  //==========================================================================
  // This is the version of strongly connected components from
  // "Intro. to Algorithms" by Cormen, Leiserson, Rivest, which was
  // adapted from "Data Structure and Algorithms" by Aho, Hopcroft,
  // and Ullman, who credit the algorithm to S.R. Kosaraju and M. Sharir.
  // The algorithm is based on computing DFS forests the graph
  // and its transpose.

  // This algorithm is slower than Tarjan's by a constant factor, uses
  // more memory, and puts more requirements on the graph type.

  template <class Graph, class DFSVisitor, class ComponentsMap,
            class DiscoverTime, class FinishTime,
            class ColorMap>
  typename property_traits<ComponentsMap>::value_type
  kosaraju_strong_components(Graph& G, ComponentsMap c,
                             FinishTime finish_time, ColorMap color)
  {
    BOOST_CONCEPT_ASSERT(( MutableGraphConcept<Graph> ));
    // ...
    
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;
    typename property_traits<FinishTime>::value_type time = 0;
    depth_first_search
     (G, make_dfs_visitor(stamp_times(finish_time, time, on_finish_vertex())),
      color);

    Graph G_T(num_vertices(G));
    transpose_graph(G, G_T);

    typedef typename property_traits<ComponentsMap>::value_type count_type;

    count_type c_count(0);
    detail::components_recorder<ComponentsMap>
      vis(c, c_count);

    // initialize G_T
    typename graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(G_T); ui != ui_end; ++ui)
      put(color, *ui, Color::white());

    typedef typename property_traits<FinishTime>::value_type D;
    typedef indirect_cmp< FinishTime, std::less<D> > Compare;

    Compare fl(finish_time);
    std::priority_queue<Vertex, std::vector<Vertex>, Compare > Q(fl);

    typename graph_traits<Graph>::vertex_iterator i, j, iend, jend;
    boost::tie(i, iend) = vertices(G_T);
    boost::tie(j, jend) = vertices(G);
    for ( ; i != iend; ++i, ++j) {
      put(finish_time, *i, get(finish_time, *j));
       Q.push(*i);
    }

    while ( !Q.empty() ) {
      Vertex u = Q.top();
      Q.pop();
      if  (get(color, u) == Color::white()) {
        depth_first_visit(G_T, u, vis, color);
        ++c_count; 
      }
    }
    return c_count;
  }

} // namespace boost

#ifdef BOOST_GRAPH_USE_MPI
#  include <boost/graph/distributed/strong_components.hpp>
#endif

#endif // BOOST_GRAPH_STRONG_COMPONENTS_HPP
