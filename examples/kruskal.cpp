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
#include <vector>

#include <iostream>
#include <iterator>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_minimum_spanning_tree.hpp>
/*

  Sample output:

  Print the edge in MST:
  0 <--> 2 with weight of 1
  3 <--> 4 with weight of 1
  4 <--> 0 with weight of 1
  1 <--> 3 with weight of 1
*/


int main(int argc, char* argv[]) 
{
  using namespace boost;
  using namespace std;

  typedef plugin<weight_tag,int> weightp;
  typedef adjacency_list< vecS, vecS, undirectedS, 
                no_plugin, weightp > Graph;
  typedef Graph::edge_descriptor Edge;
  typedef Graph::vertex_descriptor Vertex;

  typedef std::pair<int,int> E;
  const int num_nodes = 5;
  E edges[] = { E(0,2), 
		E(1,1), E(1,3), E(1,4),
		E(2,1), E(2,3), 
		E(3,4),
		E(4,0), E(4,1) };
  int weights[] = { 1, 
		    2, 1, 2,
		    7, 3, 
		    1,
		    1, 1};

  Graph G(num_nodes, edges, edges + sizeof(edges)/sizeof(E), weights);
  edge_property_accessor<Graph,weight_tag>::type weight
    = get_edge_property_accessor(G, weight_tag());

  typedef std::vector<Edge> container;
  std::vector<Edge> spanning_tree_edges;
  spanning_tree_edges.reserve(num_vertices(G));
  std::vector<Vertex> p(num_vertices(G));
  std::vector<int> rank(num_vertices(G));

  kruskal_minimum_spanning_tree(G, std::back_inserter(spanning_tree_edges), 
				rank.begin(), p.begin());
  
  cout << "Print the edge in MST:" << endl;
  for (std::vector<Edge>::iterator ei = spanning_tree_edges.begin();
       ei != spanning_tree_edges.end(); ++ei) {
    cout << source(*ei, G) << " <--> " 
	 << target(*ei, G)
	 << " with weight of " << weight[*ei]
	 << endl;
    }

  return 0;
}
