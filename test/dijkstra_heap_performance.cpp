// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_DIJKSTRA_TESTING_DIETMAR
#  define BOOST_GRAPH_DIJKSTRA_TESTING
#endif

#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths_no_color_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/timer/timer.hpp>
#include <vector>
#include <iostream>

#include <iterator>
#include <utility>
#include <boost/random/uniform_int.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/type_traits/is_base_and_derived.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/detail/lightweight_test.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost;


struct show_events_visitor : dijkstra_visitor<>
{
  template<typename Vertex, typename Graph>
  void discover_vertex(Vertex v, const Graph&)
  {
    std::cerr << "on_discover_vertex(" << v << ")\n";
  }

  template<typename Vertex, typename Graph>
  void examine_vertex(Vertex v, const Graph&)
  {
    std::cerr << "on_discover_vertex(" << v << ")\n";
  }
};


template<typename Graph, typename Kind>
void run_test(const Graph& g, const char* name, Kind kind,
              const std::vector<double>& correct_distances)
{
  std::vector<double> distances(num_vertices(g));

  // dijkstra_heap_kind = kind;

  dijkstra_shortest_paths(g, vertex(0, g),
                          distance_map(&distances[0]).
                          visitor(show_events_visitor()));
  // std::cout << run_time << " seconds.\n";

  BOOST_TEST(distances == correct_distances);
}


int main(int argc, char* argv[])
{
  unsigned n = (argc > 1? lexical_cast<unsigned>(argv[1]) : 10000u);
  unsigned m = (argc > 2? lexical_cast<unsigned>(argv[2]) : 10*n);
  int seed = (argc > 3? lexical_cast<int>(argv[3]) : 1);

  // Build random graph
  typedef adjacency_list<vecS, vecS, directedS, no_property,
                         property<edge_weight_t, double> > Graph;
  minstd_rand gen(seed);
  double p = double(m)/(double(n)*double(n));
  Graph g(erdos_renyi_iterator<minstd_rand, Graph>(gen, n, p),
          erdos_renyi_iterator<minstd_rand, Graph>(),
          n);

  uniform_real<double> rand01(0.0, 1.0);
  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    put(edge_weight, g, *ei, rand01(gen));

  std::vector<double> binary_heap_distances(n);
  std::vector<double> relaxed_heap_distances(n);
  std::vector<double> no_color_map_distances(n);

  // Run binary or d-ary heap version
  dijkstra_shortest_paths(g, vertex(0, g),
                          distance_map(
                            boost::make_iterator_property_map(
                              binary_heap_distances.begin(), get(boost::vertex_index, g))));

  // Run relaxed heap version

  dijkstra_shortest_paths(g, vertex(0, g),
                          distance_map(
                            boost::make_iterator_property_map(
                              relaxed_heap_distances.begin(), get(boost::vertex_index, g))));

  // Verify that the results are equivalent
  BOOST_TEST(binary_heap_distances == relaxed_heap_distances);

  // Run Michael's no-color-map version
  dijkstra_shortest_paths_no_color_map
    (g, vertex(0, g),
     boost::dummy_property_map(),
     boost::make_iterator_property_map(no_color_map_distances.begin(),
                                       get(boost::vertex_index, g),
                                       0.),
     get(boost::edge_weight, g),
     get(boost::vertex_index, g),
     std::less<double>(),
     boost::closed_plus<double>(),
     (std::numeric_limits<double>::max)(),
     0,
     make_dijkstra_visitor(null_visitor())
     );

  // Verify that the results are equivalent
  BOOST_TEST(binary_heap_distances == no_color_map_distances);

  return boost::report_errors();
}
