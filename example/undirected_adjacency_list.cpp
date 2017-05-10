//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include "range_pair.hpp"

using namespace boost;

template <typename UndirectedGraph> void
undirected_graph_demo1()
{
  const int V = 3;
  UndirectedGraph undigraph(V);

  auto zero = vertex(0, undigraph);
  auto one = vertex(1, undigraph);
  auto two = vertex(2, undigraph);
  add_edge(zero, one, undigraph);
  add_edge(zero, two, undigraph);
  add_edge(one, two, undigraph);

  std::cout << "out_edges(0):";
  for (const auto& edge : make_range_pair(out_edges(zero, undigraph)))
    std::cout << ' ' << edge;
  std::cout << std::endl << "in_edges(0):";
  for (const auto& edge : make_range_pair(in_edges(zero, undigraph)))
    std::cout << ' ' << edge;
  std::cout << std::endl;
}

template <typename DirectedGraph> void
directed_graph_demo()
{
  const int V = 2;
  DirectedGraph digraph(V);
  using Weight = typename DirectedGraph::edge_property_type;
  auto weight = get(edge_weight, digraph);

  auto u = vertex(0, digraph);
  auto v = vertex(1, digraph);
  add_edge(u, v, Weight(1.2), digraph);
  add_edge(v, u, Weight(2.4), digraph);
  auto [e1, found1] = edge(u, v, digraph);
  auto [e2, found2] = edge(v, u, digraph);
  std::cout << "in a directed graph is ";
#ifdef __GNUC__
  // no boolalpha
  std::cout << "(u,v) == (v,u) ? " << (e1 == e2) << std::endl;
#else
  std::cout << "(u,v) == (v,u) ? "
    << std::boolalpha << (e1 == e2) << std::endl;
#endif
  std::cout << "weight[(u,v)] = " << get(weight, e1) << std::endl;
  std::cout << "weight[(v,u)] = " << get(weight, e2) << std::endl;
}

template <typename UndirectedGraph> void
undirected_graph_demo2()
{
  const int V = 2;
  UndirectedGraph undigraph(V);
  using Weight = typename UndirectedGraph::edge_property_type;
  typename property_map<UndirectedGraph, edge_weight_t>::type
    weight = get(edge_weight, undigraph);

  auto u = vertex(0, undigraph);
  auto v = vertex(1, undigraph);
  add_edge(u, v, Weight(3.1), undigraph);
  auto [e1, found1] = edge(u, v, undigraph);
  auto [e2, found2] = edge(v, u, undigraph);
  std::cout << "in an undirected graph is ";
#ifdef __GNUC__
  std::cout << "(u,v) == (v,u) ? " << (e1 == e2) << std::endl;
#else
  std::cout << "(u,v) == (v,u) ? "
    << std::boolalpha << (e1 == e2) << std::endl;
#endif
  std::cout << "weight[(u,v)] = " << get(weight, e1) << std::endl;
  std::cout << "weight[(v,u)] = " << get(weight, e2) << std::endl;

  std::cout << "the edges incident to v: ";
  auto s = vertex(0, undigraph);
  for (const auto& edge : make_range_pair(out_edges(s, undigraph)))
    std::cout << "(" << source(edge, undigraph) 
              << "," << target(edge, undigraph) << ")" << std::endl;
}


int
main()
{
  using Weight = property < edge_weight_t, double >;
  using UndirectedGraph = adjacency_list < vecS, vecS, undirectedS,
    no_property, Weight >;
  using DirectedGraph = adjacency_list < vecS, vecS, directedS,
    no_property, Weight >;
  undirected_graph_demo1<UndirectedGraph> ();
  directed_graph_demo<DirectedGraph> ();
  undirected_graph_demo2<UndirectedGraph> ();
  return 0;
}
