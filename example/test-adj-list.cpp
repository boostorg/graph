//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University,
// Bloomington, IN 47405.
//
// Permission to modify the code and to distribute the code is
// granted, provided the text of this NOTICE is retained, a notice if
// the code was modified is included with the above COPYRIGHT NOTICE
// and with the COPYRIGHT NOTICE in the LICENSE file, and that the
// LICENSE file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
// need to fixed test-mutable-graph.w, update it to changes
// in the concept docs, names of parts, etc.
// test-mutable-graph.w#test mutable graph

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <string>

int
main(int argc, char *argv[])
{
  int V = 10;
  int E = 50;
  int T = 5;
  if (argc > 2) {
    V = atoi(argv[1]);
    E = atoi(argv[2]);
    T = atoi(argv[3]);
  }
  using namespace boost;
  for (int i = 0; i < T; ++i) {
    // test adjacency list without properties
    {
      adjacency_list < vecS, vecS, directedS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < vecS, listS, undirectedS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < vecS, vecS, bidirectionalS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
      test_mutable_bidir_graph(g);
    }
    {
      adjacency_list < setS, listS, directedS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < setS, vecS, undirectedS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < setS, listS, bidirectionalS > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
      test_mutable_bidir_graph(g);
    }
#if 0
    // test adjacency list with properties
    typedef property < vertex_name_t, std::string > VertexProperties;
    typedef property < edge_name_t, std::string > EdgeProperties;
    {
      adjacency_list < vecS, vecS, directedS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < vecS, listS, undirectedS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < vecS, vecS, bidirectionalS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
      test_mutable_bidir_graph(g);
    }
    {
      adjacency_list < setS, listS, directedS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < setS, vecS, undirectedS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
    }
    {
      adjacency_list < setS, listS, bidirectionalS, VertexProperties,
        EdgeProperties > g;
      generate_random_graph(g, V, E);
      test_mutable_graph(g);
      test_mutable_bidir_graph(g);
    }
#endif
  }                             // for
}
