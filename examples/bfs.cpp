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

#include <algorithm>
#include <vector>
#include <utility>
#include <iostream>

#include <boost/graph/visitors.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/property_accessor.hpp>

/*
  
  This examples shows how to use the breadth_first_search() GGCL
  algorithm, specifically the 3 argument variant of bfs that assumes
  the graph has a color property (plugin) stored internally.

  Two pre-defined visitors are used to record the distance of each
  vertex from the source vertex, and also to record the parent of each
  vertex. Any number of visitors can be layered and passed to a GGCL
  algorithm.

  The call to vertices(G) returns an STL-compatible container which
  contains all of the vertices in the graph.  In this example we use
  the vertices container in the STL for_each() function.

  Sample Output:

  0 --> 2 
  1 --> 1 3 4 
  2 --> 1 3 4 
  3 --> 1 4 
  4 --> 0 1 
  0 --> 2 
  1 --> 1 3 4 
  2 --> 1 3 4 
  3 --> 1 4 
  4 --> 0 1 
  distances: 0 2 1 2 2 
  parent[0] = 0
  parent[1] = 2
  parent[2] = 0
  parent[3] = 2
  parent[4] = 2

*/

template <class ParentDecorator>
struct print_parent {
  print_parent(const ParentDecorator& p_) : p(p_) { }
  template <class Vertex>
  void operator()(const Vertex& v) const {
    std::cout << "parent[" << v << "] = " <<  p[v]  << std::endl;
  }
  ParentDecorator p;
};


template <class NewGraph, class Tag>
struct graph_copier 
  : public boost::base_visitor<graph_copier<NewGraph, Tag> >
{
  typedef Tag event_filter;

  graph_copier(NewGraph& graph) : new_g(graph) { }

  template <class Edge, class Graph>
  void operator()(Edge e, Graph& g) {
    add_edge(new_g, source(e, g), target(e, g));
  }
private:
  NewGraph& new_g;
};

template <class NewGraph, class Tag>
inline graph_copier<NewGraph, Tag>
copy_graph(NewGraph& g, Tag) {
  return graph_copier<NewGraph, Tag>(g);
}

int main(int argc, char* argv[]) 
{
  typedef boost::adjacency_list< 
    boost::mapS, boost::vecS, boost::bidirectionalS,
    boost::plugin<boost::color_tag, boost::default_color_type,
        boost::plugin<boost::degree_tag, int,
          boost::plugin<boost::in_degree_tag, int,
    boost::plugin<boost::out_degree_tag, int> > > >
  > Graph;
  
  Graph G(5);
  add_edge(G, 0, 2);
  add_edge(G, 1, 1);
  add_edge(G, 1, 3);
  add_edge(G, 1, 4);
  add_edge(G, 2, 1);
  add_edge(G, 2, 3);
  add_edge(G, 2, 4);
  add_edge(G, 3, 1);
  add_edge(G, 3, 4);
  add_edge(G, 4, 0);
  add_edge(G, 4, 1);

  typedef Graph::vertex_descriptor Vertex;

  Graph G_copy(5);
  // Array to store predecessor (parent) of each vertex. This will be
  // used as a Decorator (actually, its iterator will be).
  std::vector<Vertex> p(num_vertices(G));
  typedef std::vector<Vertex>::iterator Piter;

  // Array to store distances from the source to each vertex .  We use
  // a built-in array here just for variety. This will also be used as
  // a Decorator.  
  boost::graph_traits<Graph>::vertices_size_type d[5];
  std::fill_n(d, 5, 0);

  // The source vertex
  Vertex s = *(vertices(G).first);

  boost::breadth_first_search(G, s, 
   make_bfs_visitor(
    std::make_pair(boost::record_distances(d, boost::on_tree_edge()),
    std::make_pair(boost::record_predecessors(p.begin(), 
					      boost::on_tree_edge()),
		   copy_graph(G_copy, boost::on_examine_edge())))) );

  print_graph(G);
  print_graph(G_copy);

  if (num_vertices(G) < 11) {
    std::cout << "distances: ";
#ifdef BOOST_OLD_STREAM_ITERATORS
    std::copy(d, d + 5, std::ostream_iterator<int, char>(std::cout, " "));
#else
    std::copy(d, d + 5, std::ostream_iterator<int>(std::cout, " "));
#endif
    std::cout << std::endl;

    for_each(vertices(G).first, vertices(G).second, 
             print_parent<Piter>(p.begin()));
  }

  return 0;
}
