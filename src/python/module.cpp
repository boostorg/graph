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
#include "generators.hpp"

namespace boost { namespace graph { namespace python {

extern void export_Graph();
extern void export_Digraph();
extern void export_graphviz();
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
extern void export_cuthill_mckee_ordering();
extern void export_king_ordering();
extern void export_minimum_degree_ordering();
extern void export_sequential_vertex_coloring();
extern void export_betweenness_centrality();
extern void export_page_rank();
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
  using boost::python::no_init;
  using boost::python::init;
  using boost::python::arg;

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

  class_<erdos_renyi>("ErdosRenyi", no_init)
    .def(init<std::size_t, double>(
           (arg("n"), arg("probability") = 1)))
    ;

  class_<power_law_out_degree>("PowerLawOutDegree", no_init)
    .def(init<std::size_t, double, double>(
           (arg("n"), arg("alpha"), arg("beta"))))
    ;

  class_<small_world>("SmallWorld", no_init)
    .def(init<std::size_t, std::size_t, double>
           ((arg("n"), arg("k"), arg("probability"))))
    ;

  export_Graph();
  export_Digraph();
  export_graphviz();
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

  // Sparse Matrix Ordering
  export_cuthill_mckee_ordering();
  export_king_ordering();
  export_minimum_degree_ordering();

  // Other algorithms
  export_topological_sort();
  export_transitive_closure();
  export_sequential_vertex_coloring();
  export_betweenness_centrality();
  export_page_rank();

  // Layout Algorithms
  export_circle_graph_layout();
  export_fruchterman_reingold_force_directed_layout();
  export_kamada_kawai_spring_layout();

  //  export_transpose_graph();
  export_isomorphism();
}

template void export_in_graph<Graph>();
template void export_in_graph<Digraph>();

} } } // end namespace boost::graph::python
