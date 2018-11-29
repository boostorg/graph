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
#include <iterator>
#include <vector>
#include <algorithm>
#include <utility>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/incremental_components.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/utility.hpp>
#include <boost/graph/graph_utility.hpp>

/*

  This example demonstrates the usage of the
  connected_components_on_edgelist algorithm. This differs from the
  connect_components algorithm in that the graph object
  only needs to provide access to the "list" of edges (via the
  edges() function).

  The example graphs come from "Introduction to
  Algorithms", Cormen, Leiserson, and Rivest p. 87 (though we number
  the vertices from zero instead of one).

  Sample output:

  An undirected graph (edge list):
  (0,1) (1,4) (4,0) (2,5) 
  Total number of components: 3
  Vertex 0 is in the component who's representative is 1
  Vertex 1 is in the component who's representative is 1
  Vertex 2 is in the component who's representative is 5
  Vertex 3 is in the component who's representative is 3
  Vertex 4 is in the component who's representative is 1
  Vertex 5 is in the component who's representative is 5

  component 0 contains: 4 1 0 
  component 1 contains: 3 
  component 2 contains: 5 2 
  
 */


using boost::tie;

int main(int , char* []) 
{
  using namespace boost;
  using Index = int; // ID of a Vertex
  using Edge = std::pair<Index,Index>;
  const int N = 6;
  const int E = 4;
  Edge edgelist[] = { Edge(0, 1), Edge(1, 4), Edge(4, 0), Edge(2, 5) };
  


  edge_list<Edge*,Edge,ptrdiff_t,std::random_access_iterator_tag> g(edgelist, edgelist + E);
  std::cout << "An undirected graph (edge list):" << std::endl;
  print_edges(g, identity_property_map());
  std::cout << std::endl;

  disjoint_sets_with_storage<> ds(N);
  incremental_components(g, ds);
  
  component_index<int> components(&ds.parents()[0], 
                                  &ds.parents()[0] + ds.parents().size());

  std::cout << "Total number of components: " << components.size() << std::endl;
  for (int k = 0; k != N; ++k)
    std::cout << "Vertex " << k << " is in the component who's representative is "
         << ds.find_set(k) << std::endl;
  std::cout << std::endl;

  for (std::size_t i = 0; i < components.size(); ++i) {
    std::cout << "component " << i << " contains: ";
    component_index<int>::component_iterator
      j = components[i].first,
      jend = components[i].second;
    for ( ; j != jend; ++j)
      std::cout << *j << " ";
    std::cout << std::endl;
  }

  return 0;
}
