//=======================================================================
// Copyright 1997-2001 University of Notre Dame.
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
#include <boost/graph/tarjan_scc.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/graph_utility.hpp>

int main(int, char*[])
{
  using namespace boost;
  const char* name = "abcdefghij";

  GraphvizGraph G;
  read_graphviz("scc2.dot", G);

  typedef graph_traits<GraphvizGraph>::vertex_descriptor Vertex;
    
  std::vector<int> comp(num_vertices(G)), dfs_numbers(num_vertices(G));
  std::vector<default_color_type> color(num_vertices(G));
  std::vector<Vertex> root(num_vertices(G));
  int num = tarjan_scc(G, &comp[0], &root[0], &color[0], &dfs_numbers[0]);
    
  cout << "A directed graph:" << endl;
  print_graph(G, name);
  cout << endl;
  cout << "Total number of components: " << num << endl;
  std::vector<int>::iterator i;
  for (i = comp.begin(); i != comp.end(); ++i)
    cout << "Vertex " << name[i - comp.begin()]
	 <<" is in component " << *i << endl;
    
  return 0;
}
