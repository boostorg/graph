//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Generic Graph Component Library
//
// You should have received a copy of the License Agreement for the
// Generic Graph Component Library along with the software;  see the
// file LICENSE.  If not, contact Office of Research, University of Notre
// Dame, Notre Dame, IN  46556.
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
//

#include <iostream>
#include <vector>
#include <string>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_map.hpp>


/*
  Example of using a visitor with the depth first search 
    and breadth first search algorithm

  Sacramento ---- Reno ---- Salt Lake City
     |
  San Francisco
     |
  San Jose ---- Fresno
     |
  Los Angeles ---- Los Vegas ---- Pheonix
     |
  San Diego  


  The visitor has three main functions: 
  
  discover(u) is invoked when the algorithm first arrives at the
    vertex u. This will happen in the depth first or breadth first
    order depending on which algorithm you use.

  process(e) is invoked when the algorithm first checks an edge to see
    whether it has already been there. Whether using BFS or DFS, all
    the edges of vertex u are examined immediately after the call to
    visit(u).

  finish(u) is called when after all the vertices reachable from vertex
    u have already been visited.    

 */

using namespace std;
using namespace boost;

struct city_visitor
 : public null_visitor
{

  city_visitor(string* n) : names(n) { }

  template <class Vertex>
  inline void discover(Vertex u) {
    cout << endl << "arriving at " << names[u] << endl
	 << "  neighboring cities are: ";
  }

  template <class Edge, class Graph>
  inline void process(Edge e, Graph& g) {
    cout << names[ target(e,g) ] << ", ";
  }

  template <class Vertex>
  inline void finish(Vertex u) {
    cout << endl << "finished with " << names[u] << endl;
  }

  string* names;
};



int main(int argc, char* argv[]) 
{

  enum { SanJose, SanFran, LA, SanDiego, Fresno, LosVegas, Reno,
	 Sacramento, SaltLake, Pheonix, N };

  string names[] = { "San Jose", "San Francisco",  "San Jose",
                     "San Francisco", "Los Angeles", "San Diego", 
                     "Fresno", "Los Vegas", "Reno", "Sacramento",
                     "Salt Lake City", "Pheonix" };

  typedef std::pair<int,int> E;
  E edge_array[] = { E(Sacramento, Reno), E(Sacramento, SanFran),
		     E(Reno, SaltLake),
		     E(SanFran, SanJose),
		     E(SanJose, Fresno), E(SanJose, LA),
		     E(LA, LosVegas), E(LA, SanDiego),
		     E(LosVegas, Pheonix) };

  /* Create the graph type we want. */
  typedef adjacency_list<vecS, vecS, undirectedS> Graph;
  Graph G(N, edge_array, edge_array + sizeof(edge_array)/sizeof(E));

  /* DFS and BFS need to "color" the vertices */
  typedef vector<default_color_type> colors_t;
  colors_t colors(N);

  cout << "*** Depth First ***" << endl;
  depth_first_search(G, city_visitor(names), colors.begin());
  cout << endl;

  /* Get the source vertex */
  boost::graph_traits<Graph>::vertex_descriptor 
    s = vertex(SanJose,G);

  cout << "*** Breadth First ***" << endl;
  breadth_first_search(G, s, city_visitor(names), colors.begin());

  //end
  return 0;
}
