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
// This is a demonstration of using Johnson's algorithm for solving
// the all-pairs shortest-paths problem. The graph is from figure 36.1
// page 556 of the CLR.
//
//    Sample output:
//
//              0       1       2       3       4       5       
//    0 ->      0       0       -1      -5      0       -4      
//    1 ->      inf     0       1       -3      2       -4      
//    2 ->      inf     3       0       -4      1       -1      
//    3 ->      inf     7       4       0       5       3       
//    4 ->      inf     2       -1      -5      0       -2      
//    5 ->      inf     8       5       1       6       0       

#include <iostream>
#include <vector>
#include <iomanip>
#include <boost/property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/johnson_all_pairs_shortest_paths.hpp>

int
main()
{
  using namespace boost;

  typedef adjacency_list<vecS, vecS, directedS, no_property,
    property<edge_weight_t, int> > Graph;
  const int V = 6;

  typedef std::pair<int,int> Edge;
  Edge edge_array[] = { Edge(0,1), Edge(0,2), Edge(0,3), Edge(0,4), Edge(0,5),
                        Edge(1, 2), Edge(1,5), Edge(1,3),
                        Edge(2, 4), Edge(2,5),
                        Edge(3, 2), 
                        Edge(4, 3), Edge(4,1),
                        Edge(5, 4) };
  const int E = sizeof(edge_array)/sizeof(Edge);

  Graph g(V, edge_array, edge_array + E);

  property_map<Graph,edge_weight_t>::type w = get(edge_weight, g);

  int weights[] = { 0, 0, 0, 0, 0,
                    3, -4, 8,
                    1, 7,
                    4,
                    -5, 2,
                    6 };
  int* wp = weights;
  
  Graph::edge_iterator e,e_end;
  for (boost::tie(e,e_end) = edges(g); e != e_end; ++e)
    w[*e] = *wp++;

  std::vector<int> d(V, std::numeric_limits<int>::max());
  std::vector<int> h(V);
  std::vector<default_color_type> c(V);

  //int D[V][V]; //SGI MIPSpro Compiler barfs for the type of D.
  std::vector<std::vector<int> > D(6, std::vector<int>(6)); 

  johnson_all_pairs_shortest_paths(g, D, &d[0], &h[0], w, &c[0], 
                                   get(vertex_index, g));

  std::cout << "\t";
  for (int k = 0; k < V; ++k) 
    std::cout << k << "\t";
  std::cout << std::endl;
  for (int i = 0; i < V; ++i) {
    std::cout << i << " ->\t";
    for (int j = 0; j < V; ++j) {
      if (D[i][j] > 20 || D[i][j] < -20)
        std::cout << "inf\t";
      else
        std::cout << D[i][j] << "\t";
    }
    std::cout << std::endl;
  }
  
  return 0;
}
