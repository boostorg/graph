// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#include <boost/graph/kamada_kawai_spring_layout.hpp>
#include <boost/graph/circle_layout.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/test/minimal.hpp>
#include <iostream>
#include <limits>
#include <fstream>
#include <string>
using namespace boost;

enum vertex_position_t { vertex_position };
namespace boost { BOOST_INSTALL_PROPERTY(vertex, position); }

struct point
{
  double x;
  double y;
};

template<typename Graph, typename PositionMap>
void print_graph_layout(const Graph& g, PositionMap position)
{
  typename graph_traits<Graph>::vertex_iterator vi, vi_end;
  int xmin = 0, xmax = 0, ymin = 0, ymax = 0;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    if ((int)position[*vi].x < xmin) xmin = (int)position[*vi].x;
    if ((int)position[*vi].x > xmax) xmax = (int)position[*vi].x;
    if ((int)position[*vi].y < ymin) ymin = (int)position[*vi].y;
    if ((int)position[*vi].y > ymax) ymax = (int)position[*vi].y;
  }

  for (int y = ymin; y <= ymax; ++y) {
    for (int x = xmin; x <= xmax; ++x) {
      // Find vertex at this position
      typename graph_traits<Graph>::vertices_size_type index = 0;
      for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi, ++index) {
        if ((int)position[*vi].x == x && (int)position[*vi].y == y)
          break;
      }

      if (vi == vi_end) std::cout << ' ';
      else std::cout << (char)(index + 'A');
    }
    std::cout << std::endl;
  }
}

template<typename Graph, typename PositionMap>
void dump_graph_layout(std::string name, const Graph& g, PositionMap position)
{
  std::ofstream out((name + ".dot").c_str());
  out << "graph " << name << " {" << std::endl;

  typename graph_traits<Graph>::vertex_iterator vi, vi_end;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    out << "  n" << get(vertex_index, g, *vi) << "[ pos=\"" 
        << (int)position[*vi].x + 25 << ", " << (int)position[*vi].y + 25 
        << "\" ];\n";
  }

  typename graph_traits<Graph>::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    out << "  n" << get(vertex_index, g, source(*ei, g)) << " -- n"
        << get(vertex_index, g, target(*ei, g)) << ";\n";
  }
  out << "}\n";
}

template<typename Graph>
void 
test_circle_layout(Graph*, typename graph_traits<Graph>::vertices_size_type n)
{
  typedef typename graph_traits<Graph>::vertex_descriptor vertex;
  typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
  typedef typename graph_traits<Graph>::vertices_size_type vertices_size_type;
  typedef typename graph_traits<Graph>::edges_size_type edges_size_type;

  Graph g(n);

  // Initialize vertex indices
  vertex_iterator vi = vertices(g).first;
  for (vertices_size_type i = 0; i < n; ++i, ++vi) 
    put(vertex_index, g, *vi, i);

  circle_graph_layout(g, get(vertex_position, g), 10.0);

  std::cout << "Regular polygon layout with " << n << " points.\n";
  print_graph_layout(g, get(vertex_position, g));
}

struct simple_edge
{
  int first, second;
};

struct kamada_kawai_done 
{
  kamada_kawai_done() : last_delta() {}

  template<typename Graph>
  bool operator()(double delta_p, 
                  typename boost::graph_traits<Graph>::vertex_descriptor p,
                  const Graph& g,
                  bool global)
  {
    if (global) {
      double diff = fabs(last_delta - delta_p);
      last_delta = delta_p;
      return fabs(diff) < 0.01;
    } else {
      return delta_p < 0.01;
    }
  }

  double last_delta;
};

template<typename Graph>
void
test_triangle(Graph*)
{
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef typename graph_traits<Graph>::edge_descriptor edge_descriptor;

  Graph g;
  
  vertex_descriptor u = add_vertex(g); put(vertex_index, g, u, 0);
  vertex_descriptor v = add_vertex(g); put(vertex_index, g, v, 1);
  vertex_descriptor w = add_vertex(g); put(vertex_index, g, w, 2);

  edge_descriptor e1 = add_edge(u, v, g).first; put(edge_weight, g, e1, 1.0);
  edge_descriptor e2 = add_edge(v, w, g).first; put(edge_weight, g, e2, 1.0);
  edge_descriptor e3 = add_edge(w, u, g).first; put(edge_weight, g, e3, 1.0);

  circle_graph_layout(g, get(vertex_position, g), 25.0);

  bool ok = kamada_kawai_spring_layout(g, 
                                       get(vertex_position, g),
                                       get(edge_weight, g),
                                       side_length(50.0));
  BOOST_TEST(ok);

  std::cout << "Triangle layout.\n";
  print_graph_layout(g, get(vertex_position, g));
}

template<typename Graph>
void
test_cube(Graph*)
{
  enum {A, B, C, D, E, F, G, H};
  simple_edge cube_edges[12] = {
    {A, E}, {A, B}, {A, D}, {B, F}, {B, C}, {C, D}, {C, G}, {D, H}, 
    {E, H}, {E, F}, {F, G}, {G, H}
  };

  Graph g(&cube_edges[0], &cube_edges[12], 8);
  
  typedef typename graph_traits<Graph>::edge_iterator edge_iterator;
  typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;

  vertex_iterator vi, vi_end;
  int i = 0;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    put(vertex_index, g, *vi, i++);

  edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    put(edge_weight, g, *ei, 1.0);
    std::cerr << "(" << (char)(get(vertex_index, g, source(*ei, g)) + 'A') 
              << ", " << (char)(get(vertex_index, g, target(*ei, g)) + 'A')
              << ") ";
  }
  std::cerr << std::endl;

  circle_graph_layout(g, get(vertex_position, g), 25.0);

  bool ok = kamada_kawai_spring_layout(g, 
                                       get(vertex_position, g),
                                       get(edge_weight, g),
                                       side_length(50.0),
                                       kamada_kawai_done());
  BOOST_TEST(ok);

  std::cout << "Cube layout.\n";
  print_graph_layout(g, get(vertex_position, g));

  dump_graph_layout("cube", g, get(vertex_position, g));
}

template<typename Graph>
void
test_triangular(Graph*)
{
  enum {A, B, C, D, E, F, G, H, I, J};
  simple_edge triangular_edges[18] = {
    {A, B}, {A, C}, {B, C}, {B, D}, {B, E}, {C, E}, {C, F}, {D, E}, {D, G},
    {D, H}, {E, F}, {E, H}, {E, I}, {F, I}, {F, J}, {G, H}, {H, I}, {I, J}
  };

  Graph g(&triangular_edges[0], &triangular_edges[18], 10);
  
  typedef typename graph_traits<Graph>::edge_iterator edge_iterator;
  typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;

  vertex_iterator vi, vi_end;
  int i = 0;
  for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
    put(vertex_index, g, *vi, i++);

  edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    put(edge_weight, g, *ei, 1.0);
    std::cerr << "(" << (char)(get(vertex_index, g, source(*ei, g)) + 'A') 
              << ", " << (char)(get(vertex_index, g, target(*ei, g)) + 'A')
              << ") ";
  }
  std::cerr << std::endl;

  circle_graph_layout(g, get(vertex_position, g), 25.0);

  bool ok = kamada_kawai_spring_layout(g, 
                                       get(vertex_position, g),
                                       get(edge_weight, g),
                                       side_length(50.0),
                                       kamada_kawai_done());
  BOOST_TEST(ok);

  std::cout << "Triangular layout.\n";
  print_graph_layout(g, get(vertex_position, g));

  dump_graph_layout("triangular", g, get(vertex_position, g));
}

int test_main(int, char*[])
{
  typedef adjacency_list<listS, listS, undirectedS, 
                         // Vertex properties
                         property<vertex_index_t, int,
                         property<vertex_position_t, point> >,
                         // Edge properties
                         property<edge_weight_t, double> > Graph;

  test_circle_layout((Graph*)0, 5);
  test_cube((Graph*)0);
  test_triangular((Graph*)0);

  return 0;
}
