//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>
#include <iostream>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include "range_pair.hpp"

using namespace boost;


/*
  Vertex Basics

  This example demonstrates the GGCL Vertex interface.

  Sample output:

  vertices(g) = 0 1 2 3 4 
  vertex id: 0
  out-edges: (0,1) (0,2) (0,3) (0,4) 
  in-edges: (2,0) (3,0) (4,0) 
  adjacent vertices: 1 2 3 4 

  vertex id: 1
  out-edges: 
  in-edges: (0,1) (3,1) (4,1) 
  adjacent vertices: 

  vertex id: 2
  out-edges: (2,0) (2,4) 
  in-edges: (0,2) 
  adjacent vertices: 0 4 

  vertex id: 3
  out-edges: (3,0) (3,1) (3,4) 
  in-edges: (0,3) 
  adjacent vertices: 0 1 4 

  vertex id: 4
  out-edges: (4,0) (4,1) 
  in-edges: (0,4) (2,4) (3,4) 
  adjacent vertices: 0 1 


 */


/* some helper functors for output */

template <class Graph>
struct print_edge {
  print_edge(Graph& g) : G(g) { }

  using Edge = typename boost::graph_traits<Graph>::edge_descriptor;
  using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
  void operator()(Edge e) const
  {
    auto id = get(vertex_index, G);

    auto src = source(e, G);
    auto targ = target(e, G);

    std::cout << "(" << id[src] << "," << id[targ] << ") ";
  }

  Graph& G;
};

template <class Graph>
struct print_index {
  print_index(Graph& g) : G(g){ }

  using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;
  void operator()(Vertex c) const
  {
    auto id = get(vertex_index, G);
    std::cout << id[c] << " ";
  }

  Graph& G;
};


template <class Graph>
struct exercise_vertex {
  using Vertex = typename boost::graph_traits<Graph>::vertex_descriptor;

  exercise_vertex(Graph& _g) : g(_g) { }

  void operator()(Vertex v) const
  {
    auto id = get(vertex_index, g);

    std::cout << "vertex id: " << id[v] << std::endl;
    
    std::cout << "out-edges: ";
    std::for_each(out_edges(v, g).first, out_edges(v,g).second, 
             print_edge<Graph>(g));

    std::cout << std::endl;

    std::cout << "in-edges: ";
    std::for_each(in_edges(v, g).first, in_edges(v,g).second, 
             print_edge<Graph>(g));

    std::cout << std::endl;
    
    std::cout << "adjacent vertices: ";
    std::for_each(adjacent_vertices(v,g).first, 
             adjacent_vertices(v,g).second, print_index<Graph>(g));
    std::cout << std::endl << std::endl;
  }

  Graph& g;
};


int
main()
{
  using MyGraphType = adjacency_list<vecS,vecS,bidirectionalS>;

  using Pair = std::pair<int,int>;
  Pair edge_array[] = { Pair(0,1), Pair(0,2), Pair(0,3), Pair(0,4),
                          Pair(2,0), Pair(3,0), Pair(2,4), Pair(3,1), 
                          Pair(3,4), Pair(4,0), Pair(4,1) };

  /* Construct a graph using the edge_array*/
  MyGraphType g(5);
  for (const auto& edge : edge_array)
    add_edge(edge.first, edge.second, g);

  auto id = get(vertex_index, g);

  std::cout << "vertices(g) = ";
  for (const auto& vertex : make_range_pair(vertices(g)))
    std::cout << id[vertex] <<  " ";
  std::cout << std::endl;

  /* Use the STL for_each algorithm to "exercise" all
     of the vertices in the graph */
  std::for_each(vertices(g).first, vertices(g).second,
           exercise_vertex<MyGraphType>(g));

  return 0;
}
