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
#include <vector>
#include <utility>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include "range_pair.hpp"

using namespace boost;

using VertexProperty = property<vertex_color_t, default_color_type,
    property<vertex_distance_t,int,
      property<vertex_degree_t,int,
        property<vertex_in_degree_t, int,
          property<vertex_out_degree_t,int>>>>>;
using EdgeProperty = property<edge_weight_t,int>;
using Graph = adjacency_list<vecS, vecS, bidirectionalS, 
                       VertexProperty, EdgeProperty>;

template <class Graph>
void print(Graph& g) {
  for (const auto& vertex : make_range_pair(boost::vertices(g))) {
    std::cout << vertex << " --> ";
    for (const auto& edge : make_range_pair(out_edges(vertex, g)))
      std::cout << target(edge, g) << "  ";
    std::cout << std::endl;
  }
}

std::size_t myrand(std::size_t N) {
  std::size_t ret = rand() % N; 
  //  std::cout << "N = " << N << "  rand = " << ret << std::endl;
  return ret;
}

template <class Graph>
bool check_edge(Graph& g, std::size_t a, std::size_t b) {
  auto [vi, viend] = adjacent_vertices(vertex(a,g), g);

  auto found = find(vi, viend, vertex(b, g));
  if ( found == viend )
    return false;

  return true;
}

int main(int, char*[])
{
  std::size_t N = 5;

  Graph g(N);
  int i;

  bool is_failed = false;

  for (i=0; i<6; ++i) {
    std::size_t a = myrand(N), b = myrand(N);
    while ( a == b ) b = myrand(N);
    std::cout << "edge edge (" << a << "," << b <<")" << std::endl;
    //add edges
    add_edge(a, b, g);
    is_failed =  is_failed || (! check_edge(g, a, b) );
  }
  
  if ( is_failed )
    std::cerr << "    Failed."<< std::endl;
  else
    std::cerr << "           Passed."<< std::endl;
  
  print(g);
  
  //remove_edge
  for (i = 0; i<2; ++i) {
    std::size_t a = myrand(N), b = myrand(N);
    while ( a == b ) b = myrand(N);
    std::cout << "remove edge (" << a << "," << b <<")" << std::endl;
    remove_edge(a, b, g);
    is_failed = is_failed || check_edge(g, a, b);
  }
  if ( is_failed )
    std::cerr << "    Failed."<< std::endl;
  else
    std::cerr << "           Passed."<< std::endl;

  print(g);
  
  //add_vertex
  is_failed = false;
  std::size_t old_N = N;
  std::size_t vid   = add_vertex(g);
  std::size_t vidp1 = add_vertex(g);
  
  N = num_vertices(g);
  if ( (N - 2) != old_N )
    std::cerr << "    Failed."<< std::endl;
  else
    std::cerr << "           Passed."<< std::endl;      
  
  is_failed = false;
  for (i=0; i<2; ++i) {
    std::size_t a = myrand(N), b = myrand(N);
    while ( a == vid ) a = myrand(N);
    while ( b == vidp1 ) b = myrand(N);
    std::cout << "add edge (" << vid << "," << a <<")" << std::endl;
    std::cout << "add edge (" << vid << "," << vidp1 <<")" << std::endl;
    add_edge(vid, a, g);
    add_edge(b, vidp1, g);
    is_failed = is_failed || ! check_edge(g, vid, a);
    is_failed = is_failed || ! check_edge(g, b, vidp1);
  }
  if ( is_failed )
    std::cerr << "    Failed."<< std::endl;
  else
    std::cerr << "           Passed."<< std::endl;
  print(g);
  
  // clear_vertex
  std::size_t c = myrand(N);
  is_failed = false;
  clear_vertex(c, g);

  if ( out_degree(c, g) != 0 )
    is_failed = true;

  std::cout << "Removing vertex " << c << std::endl;
  remove_vertex(c, g);
  
  old_N = N;
  N = num_vertices(g);
  
  if ( (N + 1) != old_N )
    is_failed = true;
  
  if ( is_failed )
    std::cerr << "    Failed."<< std::endl;
  else
    std::cerr << "           Passed."<< std::endl;      
  
  print(g);
  
  return 0;
}
