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
#ifndef BOOST_GRAPH_BREADTH_FIRST_SEARCH_HPP
#define BOOST_GRAPH_BREADTH_FIRST_SEARCH_HPP

/*
  Breadth First Search Algorithm (Cormen, Leiserson, and Rivest p. 470)
*/
#include <boost/config.hpp>
#include <boost/pending/queue.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/visitors.hpp>

namespace boost {

  template <class Visitor, class Graph>
  struct BFSVisitorConcept {
    void constraints() {
      vis.initialize_vertex(u, g);
      vis.start_vertex(u, g);
      vis.discover_vertex(u, g);
      vis.examine_edge(e, g);
      vis.tree_edge(e, g);
      vis.cycle_edge(e, g);
      vis.gray_target(e, g);
      vis.black_target(e, g);
      vis.finish_vertex(u, g);
    }
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::vertex_descriptor u;
    typename graph_traits<Graph>::edge_descriptor e;
  };

  // Variant (1)
  template <class VertexListGraph, class BFSVisitor>
  inline void breadth_first_search(VertexListGraph& g,
    typename graph_traits<VertexListGraph>::vertex_descriptor s, 
    BFSVisitor vis)
  {
    breadth_first_search(g, s, vis, get(vertex_color, g));
  }

  // Variant (2)
  template <class VertexListGraph, class BFSVisitor, class ColorMap>
  inline void breadth_first_search(VertexListGraph& g,
    typename graph_traits<VertexListGraph>::vertex_descriptor s, 
    BFSVisitor vis, ColorMap color)
  {
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
    boost::queue<Vertex> Q;
    typename property_traits<ColorMap>::value_type c = get(color, s);

    typename boost::graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      put(color, *ui, white(c));
      vis.initialize_vertex(*ui, g);
    }
    breadth_first_search(g, s, Q, vis, color);
  }

  // Variant (3)
  template <class IncidenceGraph, class Buffer, class BFSVisitor, 
            class ColorMap>
  inline void breadth_first_search(IncidenceGraph& g, 
    typename graph_traits<IncidenceGraph>::vertex_descriptor s, 
    Buffer& Q, BFSVisitor vis, ColorMap color)
  {
    function_requires< IncidenceGraphConcept<IncidenceGraph> >();
    typedef graph_traits<IncidenceGraph> GTraits;
    typedef typename GTraits::vertex_descriptor Vertex;
    typedef typename GTraits::edge_descriptor Edge;
    function_requires< BFSVisitorConcept<BFSVisitor, IncidenceGraph> >();
    function_requires< ReadWritePropertyMapConcept<ColorMap, Vertex> >();
    typename property_traits<ColorMap>::value_type c = get(color, s);

    put(color, s, gray(c));
    vis.start_vertex(s, g);
    Q.push(s);
    while (! Q.empty()) {
      Vertex u = Q.top();
      Q.pop(); // pop before push to avoid problem if Q is priority_queue.
      typename GTraits::out_edge_iterator ei, ei_end;
      for (tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
        Edge e = *ei;
        vis.examine_edge(e, g);
        Vertex v = target(e, g);
        if (get(color, v) == white(c)) {
          put(color, v, gray(c));
          vis.discover_vertex(u, g);
          vis.tree_edge(e, g);
          Q.push(v);
        } else {
          vis.cycle_edge(e, g);

          if (get(color, v) == gray(c))
            vis.gray_target(e, g);
          else
            vis.black_target(e, g);
        }
      } // for 
      put(color, u, black(c));
      vis.finish_vertex(u, g);
    } // while
  }

  template <class Visitors = null_visitor>
  class bfs_visitor {
  public:
    bfs_visitor(Visitors vis = Visitors()) : m_vis(vis) { }

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
    void examine_edge(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_examine_edge());
    }
    template <class Edge, class Graph>
    void tree_edge(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_tree_edge());      
    }
    template <class Edge, class Graph>
    void cycle_edge(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_cycle_edge());
    }
    template <class Edge, class Graph>
    void gray_target(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_gray_target());
    }
    template <class Edge, class Graph>
    void black_target(Edge e, Graph& g) {
      invoke_visitors(m_vis, e, g, on_black_target());
    }
    template <class Vertex, class Graph>
    void finish_vertex(Vertex u, Graph& g) {
      invoke_visitors(m_vis, u, g, on_finish_vertex());      
    }
  protected:
    Visitors m_vis;
  };
  template <class Visitors>
  bfs_visitor<Visitors>
  make_bfs_visitor(Visitors vis) {
    return bfs_visitor<Visitors>(vis);
  }

} // namespace boost

#endif /* BOOST_GRAPH_BFS_H */

