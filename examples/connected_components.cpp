//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
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

#include <iostream>
#include <vector>
#include <algorithm>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/connected_components.hpp>

/*

  This example demonstrates the usage of the connected_components
  algorithm on a directed and undirected graph. The example graphs
  come from "Introduction to Algorithms", Cormen, Leiserson, and
  Rivest p. 87 (though we number the vertices from zero instead of
  one).

  Sample output:

  An undirected graph:

  Total number of components: 3
  Vertex 0 is in component 0
  Vertex 1 is in component 0
  Vertex 2 is in component 1
  Vertex 3 is in component 2
  Vertex 4 is in component 0
  Vertex 5 is in component 1

  A directed graph:

  Total number of components: 3
  Vertex 0 is in component 2
  Vertex 1 is in component 2
  Vertex 2 is in component 1
  Vertex 3 is in component 2
  Vertex 4 is in component 2
  Vertex 5 is in component 0

 */

using namespace std;

int main(int argc, char* argv[]) 
{
  // First example: the connected components of an undirected graph
  using namespace boost;
  {
    typedef property<vertex_discover_time_t, int,
      property< vertex_finish_time_t, int,
        property< vertex_color_t, default_color_type > > > VertexProperty;
    typedef adjacency_list <vecS, vecS, undirectedS, VertexProperty> Graph;
    typedef Graph::vertex_descriptor Vertex;

    const int N = 6;
    Graph G(N);
    add_edge(G, 0, 1);
    add_edge(G, 1, 4);
    add_edge(G, 4, 0);
    add_edge(G, 2, 5);
    
    
    std::vector<int> c(num_vertices(G));
    int num = connected_components(G, &c[0], get(vertex_color, G), 
                                   dfs_visitor<>());
    
    cout << "An undirected graph:" << endl;
    cout << endl;
    std::vector<int>::iterator i;
    cout << "Total number of components: " << num << endl;
    for (i = c.begin(); i != c.end(); ++i)
      cout << "Vertex " << i - c.begin() <<" is in component " << *i << endl;
    cout << endl;
  }
  // Second example: the strongly connected components of a directed
  // graph
  {
    typedef property<vertex_discover_time_t, int,
      property< vertex_finish_time_t, int,
        property< vertex_color_t, default_color_type > > > VertexProperty;
    typedef adjacency_list< vecS, vecS, directedS, VertexProperty >  Graph;
    const int N = 6;
    Graph G(N);
    add_edge(G, 0, 1);
    add_edge(G, 1, 1);
    add_edge(G, 1, 3);
    add_edge(G, 1, 4);
    add_edge(G, 4, 3);
    add_edge(G, 3, 4);
    add_edge(G, 3, 0);
    add_edge(G, 5, 2);

    typedef Graph::vertex_descriptor Vertex;
    
    std::vector<int> c(N);
    int num = connected_components(G, &c[0], get(vertex_color, G),
                                   dfs_visitor<>());
    
    cout << "A directed graph:" << endl;
    cout << endl;
    cout << "Total number of components: " << num << endl;
    std::vector<int>::iterator i;
    for (i = c.begin(); i != c.end(); ++i)
      cout << "Vertex " << i - c.begin() <<" is in component " << *i << endl;
  }

  return 0;
}

