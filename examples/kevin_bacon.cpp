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
#include <fstream>
#include <string>
#include <boost/pending/stringtok.hpp>
#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>
#include <map>

int
main()
{
  using namespace boost;
  using namespace std;
  using boost::stringtok;
  using boost::tie;

  ifstream datafile("./kevin_bacon.dat");
  if (!datafile) {
    cerr << "No ./kevin_bacon.dat file" << endl;
    return -1;
  }

  typedef adjacency_list<vecS, vecS, undirectedS, 
    plugin<vertex_name_t, string>,
    plugin<edge_name_t, string, plugin<edge_weight_t, int> >
  > Graph;

  typedef graph_traits<Graph>::vertex_descriptor Vertex;
  typedef graph_traits<Graph>::edge_descriptor Edge;

  typedef map<string, Vertex> NameVertexMap;
  NameVertexMap actors;
  Graph g;

  property_map<Graph, vertex_name_t>::type actor_name = get(vertex_name, g);
  property_map<Graph, vertex_index_t>::type vertex_id = get(vertex_index, g);
  property_map<Graph, edge_name_t>::type connecting_movie = get(edge_name, g);
  property_map<Graph, edge_weight_t>::type weight = get(edge_weight, g);
  
  string line;
  while (getline(datafile,line)) {

    list<string> line_toks;
    stringtok(line_toks, line, "|");

    NameVertexMap::iterator pos; 
    bool inserted;
    Vertex u, v;

    list<string>::iterator i = line_toks.begin();

    tie(pos, inserted) = actors.insert(make_pair(*i, Vertex()));
    if (inserted) {
      u = add_vertex(g);
      put(actor_name, u, *i);
      pos->second = u;
    } else
      u = pos->second;
    ++i;

    string movie_name = *i++;
      
    tie(pos, inserted) = actors.insert(make_pair(*i, Vertex()));
    if (inserted) {
      v = add_vertex(g);
      put(actor_name, v, *i);
      pos->second = v;
    } else
      v = pos->second;

    Edge e;
    tie(e, inserted) = add_edge(g, u, v);
    if (inserted) {
      put(connecting_movie, e, movie_name);
      put(weight, e, 1);
    }
  }

  {
    graph_traits<Graph>::edge_iterator i, end;
    for (tie(i, end) = edges(g); i != end; ++i)
      cout << get(actor_name, source(*i, g)) << " was in "
           << get(connecting_movie, *i) << " with "
           << get(actor_name, target(*i, g)) << endl;
  }

  {
    // Use Dijkstra's shortest paths algorithm to calculate the Bacon
    // numbers of all the actors in our graph.

    std::vector<int> bacon_number( num_vertices(g) );
    std::vector<default_color_type> color( num_vertices(g) );
    std::vector<Vertex> predecessor( num_vertices(g) );

    Vertex src = actors["Kevin Bacon"];
    
    dijkstra_shortest_paths
      (g, src, &bacon_number[0], get(edge_weight, g),
       &color[0], get(vertex_index, g),
       make_ucs_visitor(record_predecessors(&predecessor[0], 
                                            on_edge_relaxed())));

    graph_traits<Graph>::vertex_iterator i, end;
    for (tie(i, end) = vertices(g); i != end; ++i)
      cout << get(actor_name, *i) << "'s bacon number is " 
           << bacon_number[get(vertex_id, *i)] << endl;

    // use the predecessors to determine shortest paths to kevin...
  }
  
  return 0;
}
