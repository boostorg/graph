// Boost.Graph library isomorphism test

// Copyright (C) 2001 Doug Gregor (gregod@cs.rpi.edu)
//
// Permission to copy, use, sell and distribute this software is granted
// provided this copyright notice appears in all copies.
// Permission to modify the code and to distribute modified code is granted
// provided this copyright notice appears in all copies, and a notice
// that the code was modified is included with the copyright notice.
//
// This software is provided "as is" without express or implied warranty,
// and with no claim as to its suitability for any purpose.

// For more information, see http://www.boost.org
//
// Revision History:
//
// 29 Nov 2001    Jeremy Siek
//      Changed to use Boost.Random.
// 29 Nov 2001    Doug Gregor
//      Initial checkin.

#define BOOST_INCLUDE_MAIN
#include <boost/test/test_tools.hpp>
#include <boost/graph/adjacency_list.hpp>
//#include <boost/graph/isomorphism.hpp>
//#include <boost/graph/ddl_isomorphism.hpp>
//#include "../doc/isomorphism.hpp"
#include "isomorphism-v2.hpp"
#include <boost/property_map.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <iostream>
#include <fstream>
#include <map>
#include <algorithm>
#include <cstdlib>
#include <ctime>

using namespace boost;

// Verify that the given mapping iso_map from the vertices of g1 to the
// vertices of g2 describes an isomorphism.
// Note: this could be made much faster by specializing based on the graph
// concepts modeled, but since we're verifying an O(n^(lg n)) algorithm,
// O(n^4) won't hurt us.
template<typename Graph1, typename Graph2, typename IsoMap>
inline bool verify_isomorphism(const Graph1& g1, const Graph2& g2, 
                               IsoMap iso_map)
{
  if (num_vertices(g1) != num_vertices(g2) || num_edges(g1) != num_edges(g2))
    return false;
  
  for (typename graph_traits<Graph1>::edge_iterator e1 = edges(g1).first;
       e1 != edges(g1).second; ++e1) {
    bool found_edge = false;
    for (typename graph_traits<Graph2>::edge_iterator e2 = edges(g2).first;
         e2 != edges(g2).second && !found_edge; ++e2) {
      if (source(*e2, g2) == get(iso_map, source(*e1, g1)) &&
          target(*e2, g2) == get(iso_map, target(*e1, g1))) {
        found_edge = true;
      }
    }
    
    if (!found_edge)
      return false;
  }
  
  return true;
}

template <typename Generator>
struct random_functor {
  random_functor(Generator& g) : g(g) { }
  std::size_t operator()(std::size_t n) {
    boost::uniform_int<boost::mt19937, std::size_t> x(g, 0, n-1);
    return x();
  }
  Generator& g;
};

template<typename Graph1, typename Graph2>
void randomly_permute_graph(const Graph1& g1, Graph2& g2)
{
  // Need a clean graph to start with
  assert(num_vertices(g2) == 0);
  assert(num_edges(g2) == 0);

  typedef typename graph_traits<Graph1>::vertex_descriptor vertex1;
  typedef typename graph_traits<Graph2>::vertex_descriptor vertex2;
  typedef typename graph_traits<Graph1>::edge_iterator edge_iterator;

  boost::mt19937 gen;
  random_functor<boost::mt19937> rand_fun(gen);

  // Decide new order
  std::vector<vertex1> orig_vertices(vertices(g1).first, vertices(g1).second);
  std::random_shuffle(orig_vertices.begin(), orig_vertices.end(), rand_fun);
  std::map<vertex1, vertex2> vertex_map;

  for (std::size_t i = 0; i < num_vertices(g1); ++i) {
    vertex_map[orig_vertices[i]] = add_vertex(g2);
  }

  for (edge_iterator e = edges(g1).first; e != edges(g1).second; ++e) {
    add_edge(vertex_map[source(*e, g1)], vertex_map[target(*e, g1)], g2);
  }
}

template<typename Graph>
void generate_random_digraph(Graph& g, double edge_probability)
{
  typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
  boost::mt19937 random_gen;
  boost::uniform_real<boost::mt19937, double> 
    random_dist(random_gen, 0.0, 1.0);

  for (vertex_iterator u = vertices(g).first; u != vertices(g).second; ++u) {
    vertex_iterator v = u;
    ++v;
    for (; v != vertices(g).second; ++v) {
      if (random_dist() <= edge_probability)
        add_edge(*u, *v, g);
    }
  }
}

enum { a, b, c, d, e, f, g, h };
enum { _1, _2, _3, _4, _5, _6, _7, _8 };

void test_isomorphism(int n, double edge_probability) 
{
  typedef adjacency_list<vecS, vecS, bidirectionalS> GraphA;
  typedef adjacency_list<vecS, vecS, bidirectionalS> GraphB;

  char a_names[] = "abcdefgh";
  char b_names[] = "12345678";

  GraphA Ga(8);
  add_edge(a, d, Ga);
  add_edge(a, h, Ga);
  add_edge(b, c, Ga);
  add_edge(b, e, Ga);
  add_edge(c, f, Ga);
  add_edge(d, a, Ga);
  add_edge(d, h, Ga);
  add_edge(e, b, Ga);
  add_edge(f, b, Ga);
  add_edge(f, e, Ga);
  add_edge(g, d, Ga);
  add_edge(g, f, Ga);
  add_edge(h, c, Ga);
  add_edge(h, g, Ga);

  GraphB Gb(8);
  add_edge(_1, _6, Gb);
  add_edge(_2, _1, Gb);
  add_edge(_2, _5, Gb);
  add_edge(_3, _2, Gb);
  add_edge(_3, _4, Gb);
  add_edge(_4, _2, Gb);
  add_edge(_4, _3, Gb);
  add_edge(_5, _4, Gb);
  add_edge(_5, _6, Gb);
  add_edge(_6, _7, Gb);
  add_edge(_6, _8, Gb);
  add_edge(_7, _8, Gb);
  add_edge(_8, _1, Gb);
  add_edge(_8, _7, Gb);

  detail::isomorphism_algo<GraphA, GraphB> algo(Ga, Gb);
  algo.a_names = a_names;
  algo.b_names = b_names;
  algo.test_isomorphism();
  for (int i = 0; i < num_vertices(Ga); ++i)
    std::cout << "f(" << a_names[i] << ")=" << b_names[algo.f[i]] << std::endl;
  
  BOOST_TEST(verify_isomorphism(Ga, Gb, &algo.f[0]));

#if 0
  int v_idx = 0;
  for (graph2::vertex_iterator v = vertices(g2).first; 
       v != vertices(g2).second; ++v) {
    put(vertex_index_t(), g2, *v, v_idx++);
  }

  std::map<graph1::vertex_descriptor, graph2::vertex_descriptor> mapping;

  bool isomorphism_correct;
  clock_t start = clock();
  BOOST_TEST(isomorphism_correct = isomorphism
               (g1, g2, isomorphism_map(make_assoc_property_map(mapping))));
  clock_t end = clock();

  std::cout << "Elapsed time (clock cycles): " << (end - start) << std::endl;

  bool verify_correct;
  BOOST_TEST(verify_correct = 
             verify_isomorphism(g1, g2, make_assoc_property_map(mapping)));

  if (!isomorphism_correct || !verify_correct) {
    // Output graph 1
    {
      std::ofstream out("isomorphism_failure.bg1");
      out << num_vertices(g1) << std::endl;
      for (graph1::edge_iterator e = edges(g1).first; 
           e != edges(g1).second; ++e) {
        out << get(vertex_index_t(), g1, source(*e, g1)) << ' '
            << get(vertex_index_t(), g1, target(*e, g1)) << std::endl;
      }
    }

    // Output graph 2
    {
      std::ofstream out("isomorphism_failure.bg2");
      out << num_vertices(g2) << std::endl;
      for (graph2::edge_iterator e = edges(g2).first; 
           e != edges(g2).second; ++e) {
        out << get(vertex_index_t(), g2, source(*e, g2)) << ' '
            << get(vertex_index_t(), g2, target(*e, g2)) << std::endl;
      }
    }
  }
#endif
}

int test_main(int argc, char* argv[])
{
  if (argc < 3) {
    test_isomorphism(30, 0.45);
    return 0;
  }

  int n = atoi(argv[1]);
  double edge_prob = atof(argv[2]);
  test_isomorphism(n, edge_prob);

  return 0;
}
