// Copyright 2010 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Jeremiah Willcock
//           Andrew Lumsdaine

#ifndef BOOST_GRAPH_RANDOM_SPANNING_TREE_HPP
#define BOOST_GRAPH_RANDOM_SPANNING_TREE_HPP

#include <vector>
#include <boost/graph/loop_erased_random_walk.hpp>
#include <boost/graph/random.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost {

  namespace detail {
    // Use Wilson's algorithm (based on loop-free random walks) to generate a
    // random spanning tree.  The distribution of edges used is controlled by
    // the next_edge() function, so this version allows either weighted or
    // unweighted selection of trees.
    // Algorithm is from http://en.wikipedia.org/wiki/Uniform_spanning_tree
    template <typename Graph, typename PredMap, typename ColorMap, typename NextEdge>
    void random_spanning_tree_internal(const Graph& g, PredMap pred, ColorMap color, NextEdge next_edge) {
      typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
      typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;

      assert (num_vertices(g) >= 2); // g must also be undirected (or symmetric) and connected

      typedef color_traits<typename property_traits<ColorMap>::value_type> color_gen;
      BGL_FORALL_VERTICES_T(v, g, Graph) put(color, v, color_gen::white());

      std::vector<vertex_descriptor> path;

      vertex_descriptor s = *vertices(g).first;
      put(color, s, color_gen::black());
      put(pred, s, graph_traits<Graph>::null_vertex());

      BGL_FORALL_VERTICES_T(v, g, Graph) {
        if (get(color, v) != color_gen::white()) continue;
        loop_erased_random_walk(g, v, next_edge, color, path);
        for (typename std::vector<vertex_descriptor>::const_reverse_iterator i = path.rbegin();
             boost::next(i) != path.rend(); ++i) {
          typename std::vector<vertex_descriptor>::const_reverse_iterator j = i;
          ++j;
          assert (get(color, *j) == color_gen::gray());
          put(color, *j, color_gen::black());
          put(pred, *j, *i);
        }
      }
    }
  }

  // Compute a uniformly-distributed spanning tree on a graph.
  template <typename Graph, typename PredMap, typename ColorMap, typename Gen>
  void random_spanning_tree(const Graph& g, PredMap pred, Gen& gen, ColorMap color) {
    unweighted_random_out_edge_gen<Graph, Gen> random_oe(gen);
    detail::random_spanning_tree_internal(g, pred, color, random_oe);
  }

  // Compute a uniformly-distributed spanning tree on a graph.
  template <typename Graph, typename PredMap, typename Gen>
  void random_spanning_tree(const Graph& g, PredMap pred, Gen& gen) {
    std::vector<default_color_type> color_data(num_vertices(g));
    random_spanning_tree(g, pred, gen, make_iterator_property_map(color_data.begin(), get(vertex_index, g)));
  }

  // Compute a weight-distributed spanning tree on a graph.
  // Weighting works according to:
  // @article{Mosbah1999263,
  //   title = "Non-uniform random spanning trees on weighted graphs",
  //   journal = "Theoretical Computer Science",
  //   volume = "218",
  //   number = "2",
  //   pages = "263--271",
  //   year = "1999",
  //   note = "",
  //   issn = "0304-3975",
  //   doi = "DOI: 10.1016/S0304-3975(98)00325-9",
  //   url = "http://www.sciencedirect.com/science/article/B6V1G-3WSV1D9-P/2/06bea092e23163c4884844cde4a5e92c",
  //   author = "M. Mosbah and N. Saheb"
  // }
  template <typename Graph, typename PredMap, typename WeightMap, typename ColorMap, typename Gen>
  void weighted_random_spanning_tree(const Graph& g, PredMap pred, WeightMap weight, Gen& gen, ColorMap color) {
    weighted_random_out_edge_gen<Graph, WeightMap, Gen> random_oe(weight, gen);
    detail::random_spanning_tree_internal(g, pred, color, random_oe);
  }

  // Compute a weight-distributed spanning tree on a graph.
  template <typename Graph, typename PredMap, typename WeightMap, typename Gen>
  void weighted_random_spanning_tree(const Graph& g, PredMap pred, WeightMap weight, Gen& gen) {
    std::vector<default_color_type> color_data(num_vertices(g));
    weighted_random_spanning_tree(g, pred, weight, gen, make_iterator_property_map(color_data.begin(), get(vertex_index, g)));
  }
}

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_RANDOM_SPANNING_TREE_HPP
