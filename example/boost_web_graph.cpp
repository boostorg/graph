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
#include <boost/config.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <map>
#include <boost/pending/stringtok.hpp>
#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

template <class Distance>
class calc_distance_visitor : public boost::bfs_visitor<>
{
public:
  calc_distance_visitor(Distance d) : distance(d) { }

  template <class Graph>
  void tree_edge(typename boost::graph_traits<Graph>::edge_descriptor e,
                 Graph& g)
  {
    typename boost::graph_traits<Graph>::vertex_descriptor u, v;
    u = source(e, g);
    v = target(e, g);
    distance[v] = distance[u] + 1;
  }
private:
  Distance distance;
};


int
main()
{
  using namespace boost;

  std::ifstream datafile("./boost_web.dat");
  if (!datafile) {
    std::cerr << "No ./boost_web.dat file" << std::endl;
    return -1;
  }

  typedef adjacency_list<vecS, vecS, undirectedS, 
    property<vertex_name_t, std::string, 
      property<vertex_color_t, default_color_type> >,
    property<edge_name_t, std::string, property<edge_weight_t, int> >
  > Graph;

  typedef graph_traits<Graph> Traits;
  typedef Traits::vertex_descriptor Vertex;
  typedef Traits::edge_descriptor Edge;

  typedef std::map<std::string, Vertex> NameVertexMap;
  NameVertexMap name2vertex;
  Graph g;

  property_map<Graph, vertex_name_t>::type node_name = get(vertex_name, g);
  property_map<Graph, vertex_index_t>::type node_id = get(vertex_index, g);
  property_map<Graph, edge_name_t>::type link_name = get(edge_name, g);

  //===========================================================================
  // Read the Data File
  
  std::string line;
  while (getline(datafile,line)) {

    std::list<std::string> line_toks;
    boost::stringtok(line_toks, line, "|");

    NameVertexMap::iterator pos; 
    bool inserted;
    Vertex u, v;

    std::list<std::string>::iterator i = line_toks.begin();

    tie(pos, inserted) = name2vertex.insert(make_pair(*i, Vertex()));
    if (inserted) {
      u = add_vertex(g);
      put(node_name, u, *i);
      pos->second = u;
    } else
      u = pos->second;
    ++i;

    std::string hyperlink_name = *i++;
      
    tie(pos, inserted) = name2vertex.insert(make_pair(*i, Vertex()));
    if (inserted) {
      v = add_vertex(g);
      put(node_name, v, *i);
      pos->second = v;
    } else
      v = pos->second;

    Edge e;
    tie(e, inserted) = add_edge(u, v, g);
    if (inserted) {
      put(link_name, e, hyperlink_name);
    }
  }

#if 0
  {
    Traits::edge_iterator i, end;
    for (tie(i, end) = edges(g); i != end; ++i)
      std::cout << get(node_name, source(*i, g)) << " has link <"

                << get(link_name, *i) << "> to "
                << get(node_name, target(*i, g)) << std::endl;
  }
#endif

  typedef Traits::vertices_size_type size_type;
  typedef std::vector<size_type> IntVector;
  // Create N x N matrix for storing the shortest distances
  // between each vertex. Initialize all distances to zero.
  std::vector<IntVector> d_matrix(num_vertices(g),
                                  IntVector(num_vertices(g), 0));

  size_type i;
  for (i = 0; i < num_vertices(g); ++i) {
    calc_distance_visitor<size_type*> visitor(&d_matrix[i][0]);
    Traits::vertex_descriptor src = vertices(g).first[i];
    breadth_first_search(g, src, visitor);
  }

  size_type diameter = 0;
  for (i = 0; i < num_vertices(g); ++i)
    diameter = std::max(diameter, *std::max_element(d_matrix[i].begin(), 
                                                    d_matrix[i].end()));
  
  std::cout << "The diameter of the boost web-site graph is " << diameter
            << std::endl;

  return 0;
}
