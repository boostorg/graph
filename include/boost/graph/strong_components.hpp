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

#ifndef BOOST_GRAPH_STRONG_COMPONENTS_HPP
#define BOOST_GRAPH_STRONG_COMPONENTS_HPP

#include <stack>
#include <boost/config.hpp>
#include <boost/graph/depth_first_search.hpp>

namespace boost {

  //===========================================================================
  // This is Tarjan's algorithm for strongly connected components
  // from his paper "Depth first search and linear graph algorithms".
  // It calculates the components in a single application of DFS.
  // We implement the algorithm as a dfs-visitor.

  namespace detail {
    
    template <typename ComponentMap, typename RootMap, typename DiscoverTime,
              typename Stack, typename DFSVisitor>
    class tarjan_scc_visitor : public DFSVisitor 
    {
      typedef typename property_traits<ComponentMap>::value_type comp_type;
      typedef typename property_traits<DiscoverTime>::value_type time_type;
    public:
      tarjan_scc_visitor(ComponentMap comp_map, RootMap r, DiscoverTime d, 
                         comp_type& c_, Stack& s_, DFSVisitor v = DFSVisitor())
        : c(c_), comp(comp_map), root(r), discover_time(d),
          dfs_time(time_type()), s(s_) { }

      template <typename Graph>
      void discover_vertex(typename graph_traits<Graph>::vertex_descriptor v,
			   const Graph& g) {
        put(root, v, v);
        put(comp, v, std::numeric_limits<comp_type>::max());
        put(discover_time, v, dfs_time++);
        s.push(v);
	DFSVisitor::discover_vertex(v, g);
      }
      template <typename Graph>
      void finish_vertex(typename graph_traits<Graph>::vertex_descriptor v,
			 const Graph& g) {
        typename graph_traits<Graph>::vertex_descriptor w;
        typename graph_traits<Graph>::out_edge_iterator ei, ei_end;
        for (tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
          w = target(*ei, g);
          if (get(comp, w) == std::numeric_limits<comp_type>::max())
            put(root, v, this->min_discover_time(get(root,v), get(root,w)));
        }
        if (get(root, v) == v) {
          do {
            w = s.top(); s.pop();
            put(comp, w, c);
          } while (w != v);
          ++c;
        }
	DFSVisitor::finish_vertex(v, g);
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
    };
    
  } // namespace detail 


  // Version (2)
  template <typename Graph, typename ComponentMap, typename RootMap,
            typename ColorMap, typename DiscoverTime, typename DFSVisitor>
  typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g,    // Input
                    ComponentMap comp, // Output
                    // Internal record keeping
                    RootMap root, ColorMap color, DiscoverTime discover_time,
		    DFSVisitor v)
  {
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename property_traits<ColorMap>::value_type ColorV;
    function_requires< ColorValueConcept<ColorV> >();
    function_requires< ReadWritePropertyMapConcept<ComponentMap, Vertex> >();
    function_requires< ReadWritePropertyMapConcept<RootMap, Vertex> >();
    typedef typename property_traits<RootMap>::value_type RootV;
    function_requires< ConvertibleConcept<RootV, Vertex> >();
    function_requires< ReadWritePropertyMapConcept<DiscoverTime, Vertex> >();
    
    typename property_traits<ComponentMap>::value_type total = 0;
    
    std::stack<Vertex> s;
    detail::tarjan_scc_visitor<ComponentMap, RootMap, DiscoverTime, 
      std::stack<Vertex>, DFSVisitor > 
      vis(comp, root, discover_time, total, s, v);
    depth_first_search(g, vis, color);
    return total;
  }
  
  // Version (1)  
  template <typename Graph, typename ComponentMap, typename RootMap,
            typename ColorMap, typename DiscoverTime>
  inline typename property_traits<ComponentMap>::value_type
  strong_components(const Graph& g,    // Input
                    ComponentMap comp, // Output
                    // Internal record keeping
                    RootMap root, ColorMap color, DiscoverTime d)
  {
    return strong_components(g, comp, root, color, d, dfs_visitor<>());
  }

} // namespace boost

#include <queue>
#include <vector>
#include <boost/graph/transpose_graph.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/connected_components.hpp> // for components_recorder

namespace boost {

  //===========================================================================
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
    function_requires< MutableGraphConcept<Graph> >();
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
    detail::components_recorder<ComponentsMap, dfs_visitor<> >
      vis(c, c_count, dfs_visitor<>());

    // initialize G_T
    typename graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(G_T); ui != ui_end; ++ui)
      put(color, *ui, Color::white());

    typedef typename property_traits<FinishTime>::value_type D;
    typedef indirect_cmp< FinishTime, std::less<D> > Compare;

    Compare fl(finish_time);
    std::priority_queue<Vertex, std::vector<Vertex>, Compare > Q(fl);

    typename graph_traits<Graph>::vertex_iterator i, j, iend, jend;
    tie(i, iend) = vertices(G_T);
    tie(j, jend) = vertices(G);
    for ( ; i != iend; ++i, ++j) {
      put(f, *i, get(finish_time, *j));
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

#endif // BOOST_GRAPH_STRONG_COMPONENTS_HPP
