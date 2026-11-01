//=======================================================================
// Copyright 1997-2001 University of Notre Dame.
// Copyright 2026 Arnaud Becheler
// Authors: Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>
#include <algorithm>
#include <functional>
#include <iostream>
#include <limits>
#include <utility>
#include <vector>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <boost/range/iterator_range.hpp>

/* Output:

  distances from start vertex:
  distance(a) = 0
  distance(b) = 3
  distance(c) = 1
  distance(d) = 3
  distance(e) = 3

  min-max paths tree
  a --> c
  b -->
  c --> d
  d --> e
  e --> b

*/

int main(int, char*[])
{
    using Graph = boost::adjacency_list< boost::listS, boost::vecS, boost::directedS, boost::no_property, boost::property< boost::edge_weight_t, int > >;
    using Vertex = boost::graph_traits< Graph >::vertex_descriptor;

    using E = std::pair< int, int >;

    const char name[] = "abcde";

    const int num_nodes = 5;
    const E edges[] = { E(0, 2), E(1, 1), E(1, 3), E(1, 4), E(2, 1), E(2, 3), E(3, 4), E(4, 0), E(4, 1) };
    const int weights[] = { 1, 2, 1, 2, 7, 3, 1, 1, 1 };
    Graph G(std::begin(edges), std::end(edges), weights, num_nodes);

    auto weight_map = boost::get(boost::edge_weight, G);
    auto index_map = boost::get(boost::vertex_index, G);

    std::vector< Vertex > p(boost::num_vertices(G));
    std::vector< int > d(boost::num_vertices(G));

    const Vertex s = *boost::vertices(G).first;

    // A path costs its heaviest edge.
    const auto max_combine = [](int distance, int weight) { return (std::max)(distance, weight); };
    const auto inf = (std::numeric_limits< int >::max)();
    boost::dijkstra_shortest_paths(G, s, p.data(), d.data(), weight_map, index_map, std::less< int >(), max_combine, inf, 0, boost::default_dijkstra_visitor());

    std::cout << "distances from start vertex:" << std::endl;
    for (const auto v : boost::make_iterator_range(boost::vertices(G)))
    {
        std::cout << "distance(" << name[v] << ") = " << d[v] << std::endl;
    }
    std::cout << std::endl;

    std::cout << "min-max paths tree" << std::endl;
    boost::adjacency_list<> tree(num_nodes);

    for (const auto v : boost::make_iterator_range(boost::vertices(G)))
    {
        if (v != p[v])
        {
            boost::add_edge(p[v], v, tree);
        }
    }

    boost::print_graph(tree, name);

    return 0;
}
