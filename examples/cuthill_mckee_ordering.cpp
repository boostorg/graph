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
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/cuthill_mckee_ordering.hpp>
#include <boost/graph/properties.hpp>

  /*
    Sample Output
    degree: 
    2 4 3 4 3 4 4 2 1 1 
    Reverse Cuthill-McKee ordering starting at :6
    8 3 0 9 2 5 1 4 7 6 
    Reverse Cuthill-McKee ordering starting at :0
    9 1 4 6 7 2 8 5 3 0 
    Reverse Cuthill-McKee ordering: //choose vertex 9
    0 8 5 7 3 6 4 2 1 9 
 */
int main(int , char* [])
{
  using namespace boost;
  using namespace std;
  identity_property_map id;
  typedef adjacency_list<vecS, vecS, undirectedS, 
     property< vertex_color_t, default_color_type,
       property<vertex_degree_t,int> > > Graph;
  typedef Graph::vertex_descriptor Vertex;

  typedef std::pair<size_t, size_t> Pair;
  Pair edges[14] = { Pair(0,3), //a-d
                     Pair(0,5),  //a-f
                     Pair(1,2),  //b-c
                     Pair(1,4),  //b-e
                     Pair(1,6),  //b-g
                     Pair(1,9),  //b-j
                     Pair(2,3),  //c-d
                     Pair(2,4),  //c-e
                     Pair(3,5),  //d-f
                     Pair(3,8),  //d-i
                     Pair(4,6),  //e-g
                     Pair(5,6),  //f-g
                     Pair(5,7),  //f-h
                     Pair(6,7) }; //g-h 
  
  Graph G(10);
  for (int i=0; i<14; ++i)
    add_edge(edges[i].first, edges[i].second, G);


  Graph::vertex_iterator ui, uiend;

  property_map<Graph,vertex_degree_t>::type deg = get(vertex_degree, G);
  cout << "degree: " << endl;
  for (boost::tie(ui, uiend) = vertices(G); ui != uiend; ++ui) {
    deg[*ui] = out_degree(*ui, G);
    cout << deg[*ui] << " ";
  }
  cout << endl;

  std::vector<Vertex> iperm(num_vertices(G));
  {
    Vertex s = vertex(6, G);
    //reverse cuthill_mckee_ordering
    cuthill_mckee_ordering(G, s, iperm.rbegin(), get(vertex_color, G), 
                           get(vertex_degree, G));
    cout << "Reverse Cuthill-McKee ordering starting at :" << s << endl;
    
    for (std::vector<Vertex>::const_iterator i = iperm.begin();
         i != iperm.end(); ++i)
      cout << id[*i] << " ";
    cout << endl;
  }
  {
    Vertex s = vertex(0, G);
    //reverse cuthill_mckee_ordering
    cuthill_mckee_ordering(G, s, iperm.rbegin(), get(vertex_color, G),
                           get(vertex_degree, G));
    cout << "Reverse Cuthill-McKee ordering starting at :" << s << endl;

    for (std::vector<Vertex>::const_iterator i=iperm.begin();
       i != iperm.end(); ++i)
      cout << id[*i] << " ";
    cout << endl;
  }

  {
    //reverse cuthill_mckee_ordering
    cuthill_mckee_ordering(G, iperm.rbegin());
    
    cout << "Reverse Cuthill-McKee ordering:" << endl;
    
    for (std::vector<Vertex>::const_iterator i=iperm.begin();
       i != iperm.end(); ++i)
      cout << id[*i] << " ";
    cout << endl;
  }
  return 0;
}
