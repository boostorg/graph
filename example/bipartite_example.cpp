/**
 *
 * Copyright (c) 2010 Matthias Walter (xammy@xammy.homelinux.net)
 *
 * Authors: Matthias Walter
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/bipartite.hpp>
#include "range_pair.hpp"

using namespace boost;

/// Example to test for bipartiteness and print the certificates.

template <typename Graph>
void print_bipartite (const Graph& g)
{
  using traits = graph_traits <Graph>;

  /// Most simple interface just tests for bipartiteness. 

  bool bipartite = is_bipartite (g);

  if (bipartite)
  {
    using partition_t = std::vector <default_color_type>;
    using index_map_t = typename property_map <Graph, vertex_index_t>::type;
    using partition_map_t = iterator_property_map <partition_t::iterator, index_map_t>;

    partition_t partition (num_vertices (g));
    partition_map_t partition_map (partition.begin (), get (vertex_index, g));

    /// A second interface yields a bipartition in a color map, if the graph is bipartite.

    is_bipartite (g, get (vertex_index, g), partition_map);

    for (const auto& vertex : make_range_pair(vertices (g)))
    {
      std::cout << "Vertex " << vertex << " has color " << (get (partition_map, vertex) == color_traits <
          default_color_type>::white () ? "white" : "black") << std::endl;
    }
  }
  else
  {
    using vertex_vector_t = std::vector <typename traits::vertex_descriptor>;
    vertex_vector_t odd_cycle;

    /// A third interface yields an odd-cycle if the graph is not bipartite.

    find_odd_cycle (g, get (vertex_index, g), std::back_inserter (odd_cycle));

    std::cout << "Odd cycle consists of the vertices:";
    for (size_t i = 0; i < odd_cycle.size (); ++i)
    {
      std::cout << " " << odd_cycle[i];
    }
    std::cout << std::endl;
  }
}

int main (int argc, char **argv)
{
  using vector_graph_t = adjacency_list <vecS, vecS, undirectedS>;
  using E = std::pair <int, int>;

  /**
   * Create the graph drawn below.
   *
   *       0 - 1 - 2
   *       |       |
   *   3 - 4 - 5 - 6
   *  /      \   /
   *  |        7
   *  |        |
   *  8 - 9 - 10
   **/

  E bipartite_edges[] = { E (0, 1), E (0, 4), E (1, 2), E (2, 6), E (3, 4), E (3, 8), E (4, 5), E (4, 7), E (5, 6), E (
      6, 7), E (7, 10), E (8, 9), E (9, 10) };
  vector_graph_t bipartite_vector_graph (&bipartite_edges[0],
      &bipartite_edges[0] + sizeof(bipartite_edges) / sizeof(E), 11);

  /**
   * Create the graph drawn below.
   * 
   *       2 - 1 - 0
   *       |       |
   *   3 - 6 - 5 - 4
   *  /      \   /
   *  |        7
   *  |       /
   *  8 ---- 9
   *  
   **/

  E non_bipartite_edges[] = { E (0, 1), E (0, 4), E (1, 2), E (2, 6), E (3, 6), E (3, 8), E (4, 5), E (4, 7), E (5, 6),
      E (6, 7), E (7, 9), E (8, 9) };
  vector_graph_t non_bipartite_vector_graph (&non_bipartite_edges[0], &non_bipartite_edges[0]
      + sizeof(non_bipartite_edges) / sizeof(E), 10);

  /// Call test routine for a bipartite and a non-bipartite graph.

  print_bipartite (bipartite_vector_graph);

  print_bipartite (non_bipartite_vector_graph);

  return 0;
}
