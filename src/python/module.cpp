// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include "basic_graph.cpp"
#include "graph.hpp"
#include "digraph.hpp"
#include <boost/python.hpp>
#include "point2d.hpp"

namespace boost { namespace graph { namespace python {

extern void export_Graph();
extern void export_Digraph();
extern void export_betweenness_centrality();
extern void export_page_rank();
extern void export_breadth_first_search();
extern void export_depth_first_search();
extern void export_dijkstra_shortest_paths();
extern void export_bellman_ford_shortest_paths();
extern void export_dag_shortest_paths();
extern void export_prim_minimum_spanning_tree();
template<typename Graph> void export_breadth_first_search_in_graph();
template<typename Graph> void export_depth_first_search_in_graph();
template<typename Graph> void export_dijkstra_shortest_paths_in_graph();
template<typename Graph> void export_dag_shortest_paths_in_graph();
template<typename Graph> void export_bellman_ford_shortest_paths_in_graph();
  template<typename Graph> void export_prim_minimum_spanning_tree_in_graph();
extern void export_connected_components();
extern void export_strong_components();
extern void export_biconnected_components();
extern void export_incremental_components();
extern void export_topological_sort();
extern void export_circle_graph_layout();
extern void export_fruchterman_reingold_force_directed_layout();
extern void export_kamada_kawai_spring_layout();
extern void export_kruskal_minimum_spanning_tree();
extern void export_transitive_closure();
  //extern void export_transpose_graph();
extern void export_isomorphism();

template<typename Graph>
void export_in_graph()
{
  export_breadth_first_search_in_graph<Graph>();
  export_depth_first_search_in_graph<Graph>();
  export_dijkstra_shortest_paths_in_graph<Graph>();
  export_bellman_ford_shortest_paths_in_graph<Graph>();
  export_dag_shortest_paths_in_graph<Graph>();
  export_prim_minimum_spanning_tree_in_graph<Graph>();
}

BOOST_PYTHON_MODULE(bgl)
{
  using boost::python::class_;
  using boost::python::enum_;

  enum_<graph_file_kind>("file_kind")
    .value("adjlist", gfk_adjlist)
    .value("graphviz", gfk_graphviz)
    ;

  enum_<default_color_type>("Color")
    .value("white", color_traits<default_color_type>::white())
    .value("gray", color_traits<default_color_type>::gray())
    .value("black", color_traits<default_color_type>::black())
    ;

  class_<point2d>("Point2D")
    .def_readwrite("x", &point2d::x)
    .def_readwrite("y", &point2d::y)
    ;

  export_Graph();
  export_Digraph();
  export_betweenness_centrality();
  export_page_rank();
  // Core Algorithm Patterns
  export_breadth_first_search();
  export_depth_first_search();
  // Shortest Paths Algorithms
  export_dijkstra_shortest_paths();
  export_bellman_ford_shortest_paths();
  export_dag_shortest_paths();
  // Minimum Spanning Tree Algorithms
  export_kruskal_minimum_spanning_tree();
  export_prim_minimum_spanning_tree();
  // Connected Components Algorithms
  export_connected_components();  
  export_strong_components();  
  export_biconnected_components();  
  export_incremental_components();  
  // Layout Algorithms
  export_circle_graph_layout();
  export_fruchterman_reingold_force_directed_layout();
  export_kamada_kawai_spring_layout();
  // Other algorithms
  export_topological_sort();
  export_transitive_closure();
  //  export_transpose_graph();
  export_isomorphism();
}

template void export_in_graph<Graph>();
template void export_in_graph<Digraph>();

} } } // end namespace boost::graph::python
