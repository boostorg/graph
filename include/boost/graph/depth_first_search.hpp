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
#ifndef BOOST_GRAPH_RECURSIVE_DFS_HPP
#define BOOST_GRAPH_RECURSIVE_DFS_HPP

#include <stack>
#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/visitors.hpp>

namespace boost {

  template <class Visitor, class Graph>
  class DFSVisitor_concept {
  public:
    void constraints() {
      vis.initialize_vertex(u, g);
      vis.start_vertex(u, g);
      vis.discover_vertex(u, g);
      vis.examine_edge(e, g);
      vis.tree_edge(e, g);
      vis.back_edge(e, g);
      vis.forward_or_cross_edge(e, g);
      vis.finish_vertex(u, g);
    }
  private:
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::vertex_descriptor u;
    typename graph_traits<Graph>::edge_descriptor e;
  };
  
  // Variant (1)
  template <class Graph, class DFSVisitor>
  inline void
  depth_first_search(Graph& g, DFSVisitor v)
  {
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    depth_first_search(g, v, get(vertex_color, g));
  }

  // Variant (2)
  template <class VertexListGraph, class DFSVisitor, class ColorMap>
  void
  depth_first_search(VertexListGraph& g, DFSVisitor vis, ColorMap color)
  {
    REQUIRE2(DFSVisitor, VertexListGraph, DFSVisitor);
    typename property_traits<ColorMap>::value_type 
      c = get(color, *vertices(g).first); // value of c not used, just type

    typename graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      put(color, *ui, white(c));
      vis.initialize_vertex(*ui, g);
    }
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
      if (get(color, *ui) == white(c)) {
        vis.start_vertex(*ui, g);
        depth_first_visit(g, *ui, vis, color);
      }
  }

  template <class IncidenceGraph, class DFSVisitor, class ColorMap>
  void depth_first_visit(IncidenceGraph& g,
            typename graph_traits<IncidenceGraph>::vertex_descriptor u, 
            DFSVisitor& vis, ColorMap color)
  {
    REQUIRE(IncidenceGraph, IncidenceGraph);
    REQUIRE2(DFSVisitor, IncidenceGraph, DFSVisitor);
    typename property_traits<ColorMap>::value_type c = get(color, u);

    put(color, u, gray(c));
    vis.discover_vertex(u, g);
    typename graph_traits<IncidenceGraph>::out_edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
      vis.examine_edge(*ei, g);
      if (get(color, target(*ei, g)) == white(c)) {
        vis.tree_edge(*ei, g);
        depth_first_visit(g, target(*ei, g), vis, color);
      } else if (get(color, target(*ei, g)) == gray(c))
        vis.back_edge(*ei, g);
      else
        vis.forward_or_cross_edge(*ei, g);
    }

    put(color, u, black(c));
    vis.finish_vertex(u, g);
  }


  template <class Visitors = null_visitor>
  class dfs_visitor {
  public:
    dfs_visitor(Visitors vis = null_visitor()) : m_vis(vis) { }

    template <class Vertex, class Graph>
    void initialize_vertex(Vertex u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_initialize_vertex());      
    }
    template <class Vertex, class Graph>
    void start_vertex(Vertex u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_start_vertex());      
    }
    template <class Vertex, class Graph>
    void discover_vertex(Vertex u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_discover_vertex());      
    }
    template <class Edge, class Graph>
    void examine_edge(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_examine_edge());
    }
    template <class Edge, class Graph>
    void tree_edge(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_tree_edge());      
    }
    template <class Edge, class Graph>
    void back_edge(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_back_edge());
    }
    template <class Edge, class Graph>
    void forward_or_cross_edge(Edge u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_forward_or_cross_edge());
    }
    template <class Vertex, class Graph>
    void finish_vertex(Vertex u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_finish_vertex());      
    }
  protected:
    Visitors m_vis;
  };
  template <class Visitors>
  dfs_visitor<Visitors>
  make_dfs_visitor(Visitors vis) {
    return dfs_visitor<Visitors>(vis);
  }

} // namespace boost


#endif
