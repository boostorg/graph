//=======================================================================
// Copyright 2001 Indiana University
// Author: Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University, Bloomington,
// IN 47404.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================


#define BOOST_INCLUDE_MAIN
#include <boost/test/test_tools.hpp>

#include <boost/graph/subgraph.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/graph_test.hpp>

// UNDER CONSTRUCTION 

int test_main(int argc, char* argv[])
{
  using namespace boost;
  typedef adjacency_list<vecS, vecS, directedS,
    no_property, property<edge_index_t, std::size_t> > graph_t;
  typedef subgraph<graph_t> subgraph_t;

  mt19937 gen;
  for (int t = 0; t < 100; t += 5) {
    subgraph_t g;
    int N = t + 2;
    generate_random_graph(g, N, N * 2, gen);

    graph_test< subgraph_t > gt;
    gt.test_add_vertex(g);
    gt.test_add_edge(random_vertex(g, gen), random_vertex(g, gen), g);
    gt.test_remove_edge(random_vertex(g, gen), random_vertex(g, gen), g);
    gt.test_remove_edge(random_edge(g, gen), g);
    gt.test_clear_vertex(random_vertex(g, gen), g);
  }  
  return 0;
}
