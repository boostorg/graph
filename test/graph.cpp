//=======================================================================
// Copyright 2002 Indiana University.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>

#define VERBOSE 0

#include <boost/utility.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/random.hpp>
#include <boost/pending/indirect_cmp.hpp>

#include <boost/random/mersenne_twister.hpp>
#include <boost/core/lightweight_test.hpp>


enum vertex_id_t { vertex_id = 500 };
enum edge_id_t { edge_id = 501 };
namespace boost {
  BOOST_INSTALL_PROPERTY(vertex, id);
  BOOST_INSTALL_PROPERTY(edge, id);
}


#include "graph_type.hpp" // this provides a typedef for Graph

using namespace boost;

/*
  This program tests models of the MutableGraph concept.
 */

using std::find;


template <class Graph, class Vertex, class ID>
void
check_vertex_cleared(Graph& g, Vertex v, ID id)
{
  typename graph_traits<Graph>::vertex_iterator vi, viend;
  for (boost::tie(vi,viend) = vertices(g); vi != viend; ++vi) {
    typename graph_traits<Graph>::adjacency_iterator ai, aiend, found;
    boost::tie(ai, aiend) = adjacent_vertices(*vi, g);

    found = std::find_if(ai, aiend, [v](Vertex i){ return v == i; });

    BOOST_TEST(found == aiend);
  }
}

template <class Graph, class Edge, class EdgeID>
void 
check_edge_added(Graph& g, Edge e,
                      typename graph_traits<Graph>::vertex_descriptor a,
                      typename graph_traits<Graph>::vertex_descriptor b,
                      EdgeID edge_id, std::size_t correct_id,
                      bool inserted)
{
  BOOST_TEST(source(e, g) == a);
  BOOST_TEST(target(e, g) == b);
  BOOST_TEST(is_adjacent(g, a, b));
  BOOST_TEST(in_edge_set(g,e));
  BOOST_TEST(!(inserted && edge_id[e] != correct_id));
  BOOST_TEST(!(!inserted && edge_id[e] != edge_id[edge(a, b, g).first]));
  BOOST_TEST_EQ(num_edges(g), count_edges(g));
}


template <
  class Graph>
std::size_t 
count_edges(
  Graph& g
){
  std::size_t e = 0;
  typename boost::graph_traits<Graph>::edge_iterator ei,ei_end;
  for (boost::tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
    ++e;
  return e;
}


int 
main(
  int, 
  char* []
){
  std::size_t N = 5, E = 0;
  std::size_t old_N;

  typedef ::Graph Graph;
  Graph g;
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef boost::graph_traits<Graph>::edge_descriptor Edge;

  int i, j;
  std::size_t current_vertex_id = 0;
  std::size_t current_edge_id = 0;

  bool is_failed = false;

  property_map<Graph, vertex_id_t>::type vertex_id_map = get(vertex_id, g);

  property_map<Graph, edge_id_t>::type edge_id_map = get(edge_id, g);

  for (std::size_t k = 0; k < N; ++k)
    add_vertex(current_vertex_id++, g);

  // also need to test EdgeIterator graph constructor -JGS
  mt19937 gen;

  for (j=0; j < 10; ++j) {

    // add_edge
    for (i=0; i < 6; ++i) {
      Vertex a, b;
      a = random_vertex(g, gen);
      do {
        b = random_vertex(g, gen);
      } while ( a == b ); // don't do self edges
      Edge e;
      bool inserted;
      boost::tie(e, inserted) = add_edge(a, b, current_edge_id++, g);
      check_edge_added(g, e, a, b, edge_id_map, current_edge_id - 1, inserted);
      ++E;
    }

    // remove_edge(u, v, g)
    for (i = 0; i < 2; ++i) {
      Vertex a, b;

      Edge e = random_edge(g, gen);
      boost::tie(a,b) = boost::incident(e, g);
      --E;
      remove_edge(a, b, g);
      BOOST_TEST(!(is_failed || is_adjacent(g, a, b) || in_edge_set(g, a, b)
        || num_edges(g) != count_edges(g)));
    }

    // remove_edge(e, g)
    for (i = 0; i < 2; ++i) {
      Vertex a, b;
      Edge e = random_edge(g, gen);
      boost::tie(a,b) = boost::incident(e, g);
      --E;
      graph_traits<Graph>::edges_size_type old_E = num_edges(g);
      remove_edge(e, g);

      BOOST_TEST(!(is_failed || old_E != num_edges(g) + 1
        || num_edges(g) != count_edges(g)));
    }

    // add_vertex
    old_N = num_vertices(g);
    graph_traits<Graph>::vertex_descriptor vid = add_vertex(g),
      vidp1 = add_vertex(g);
    vertex_id_map[vid] = current_vertex_id++;
    vertex_id_map[vidp1] = current_vertex_id++;

    // make sure the two added vertices are in the graph's vertex set
    {
      BOOST_TEST(in_vertex_set(g, vid));
      BOOST_TEST(in_vertex_set(g, vidp1));
    }

    // make sure the vertices do not have any out edges yet
    {
      graph_traits<Graph>::out_edge_iterator e, e_end;
      boost::tie(e,e_end) = out_edges(vid,g);
      BOOST_TEST(e == e_end);
      boost::tie(e,e_end) = out_edges(vidp1,g);
      BOOST_TEST(e == e_end);
    }

    // make sure the vertices do not yet appear in any of the edges
    {
      graph_traits<Graph>::edge_iterator e, e_end;
      for (boost::tie(e, e_end) = edges(g); e != e_end; ++e) {
        BOOST_TEST(!(source(*e,g) == vid || target(*e,g) == vid));
        BOOST_TEST(!(source(*e,g) == vidp1 || target(*e,g) == vidp1));
      }
    }
    // Make sure num_vertices(g) has been updated
    N = num_vertices(g);
    
    BOOST_TEST_EQ((N - 2), old_N );
    // add_edge again

    for (i=0; i<2; ++i) {
      Vertex a = random_vertex(g, gen), b = random_vertex(g, gen);
      while ( a == vid ) a = random_vertex(g, gen);
      while ( b == vidp1 ) b = random_vertex(g, gen);
      Edge e;
      bool inserted;
      boost::tie(e,inserted) = add_edge(vid, a, EdgeID(current_edge_id++), g);

      check_edge_added(g, e, vid, a, edge_id_map, current_edge_id - 1, inserted);

      // add_edge without plugin
      boost::tie(e,inserted) = add_edge(b, vidp1, g);
      if (inserted)
        edge_id_map[e] = current_edge_id;
      ++current_edge_id;

      check_edge_added(g, e, b, vidp1, edge_id_map, current_edge_id - 1, inserted);
    }

    // clear_vertex
    Vertex c = random_vertex(g, gen);
    clear_vertex(c, g);
    check_vertex_cleared(g, c, vertex_id_map);
    BOOST_TEST_EQ(num_edges(g), count_edges(g));

    old_N = num_vertices(g);
    remove_vertex(c, g);
    // can't check in_vertex_set here because the vertex_descriptor c
    // is no longer valid, we'll just make sure the vertex set has
    // one fewer vertex
    {
      graph_traits<Graph>::vertex_iterator v, v_end;
      boost::tie(v, v_end) = vertices(g);
      for (N = 0; v != v_end; ++v) ++N; // N = std::distance(v, v_end);
      BOOST_TEST_EQ(N, old_N - 1);
    }

    N = num_vertices(g);
    BOOST_TEST_EQ(N, old_N - 1);
  }

  return boost::report_errors();
}
