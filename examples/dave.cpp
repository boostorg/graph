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
#include <iterator>
#include <vector>
#include <list>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

using namespace std;
using namespace boost;
/*
  This example does a best-first-search (using dijkstra's) and
  simultaneously makes a copy of the graph (assuming the graph is
  connected).

  Example Graph: (p. 90 "Data Structures and Network Algorithms", Tarjan)

              g
            3+ +2
            / 1 \
           e+----f
           |+0 5++
           | \ / |
         10|  d  |12
           |8++\7|
           +/ | +|
           b 4|  c
            \ | +
            6+|/3
              a

  Sample Output:
a --> c d 
b --> a d
c --> f
d --> c e f
e --> b g
f --> e g
g -->
Starting graph:
a(32767); c d
c(32767); f
d(32767); c e f
f(32767); e g
e(32767); b g
g(32767);
b(32767); a d
Result:
a(0); d c
d(4); f e c
c(3); f
f(9); g e
e(4); g b
g(7);
b(14); d a 

*/

typedef plugin<vertex_color, default_color_type, 
         plugin<vertex_distance,int> > VPlugin;
typedef int weight_t;
typedef plugin<edge_weight,weight_t> EPlugin;

typedef adjacency_list<vecS, vecS, directedS, VPlugin, EPlugin > Graph;



template <class Tag>
struct endl_printer
  : public boost::base_visitor< endl_printer<Tag> >
{
  typedef Tag event_filter;
  endl_printer(std::ostream& os) : m_os(os) { }
  template <class T, class Graph>
  void operator()(T, Graph&) { m_os << std::endl; }
  std::ostream& m_os;
};
template <class Tag>
endl_printer<Tag> print_endl(std::ostream& os, Tag) {
  return endl_printer<Tag>(os);
}

template <class PA, class Tag>
struct edge_printer
 : public boost::base_visitor< edge_printer<PA, Tag> >
{
  typedef Tag event_filter;

  edge_printer(PA pa, std::ostream& os) : m_pa(pa), m_os(os) { }

  template <class T, class Graph>
  void operator()(T x, Graph& g) {
    m_os << "(" << get(m_pa, source(x, g)) << "," 
	 << get(m_pa, target(x, g)) << ") ";
  }
  PA m_pa;
  std::ostream& m_os;
};
template <class PA, class Tag>
edge_printer<PA, Tag>
print_edge(PA pa, std::ostream& os, Tag) {
  return edge_printer<PA, Tag>(pa, os);
}


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

template <class Graph, class Name>
void print(Graph& G, Name name)
{
  typename boost::graph_traits<Graph>::vertex_iterator ui, uiend;
  for (boost::tie(ui,uiend)=vertices(G); ui!=uiend; ++ui) {
    cout << name[*ui] << " --> ";
    typename boost::graph_traits<Graph>::adjacency_iterator vi, viend;
    for(boost::tie(vi,viend)=adjacent_vertices(*ui,G); vi!=viend; ++vi)
      cout << name[*vi] << " ";
    cout << endl;
  }
    
}


int 
main(int argc, char* argv[])
{
  // Name and ID numbers for the vertices
  char name[] = "abcdefg";
  enum { a, b, c, d, e, f, g, N};

  Graph G(N);

  vector<weight_t> distance(N, numeric_limits<weight_t>::max());
  typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
  vector<Vertex> parent(N);

  typedef pair<int,int> E;

  E edges[] = { E(a,c), E(a,d),
                E(b,a), E(b,d),
                E(c,f),
                E(d,c), E(d,e), E(d,f),
                E(e,b), E(e,g),
                E(f,e), E(f,g) };

  int weight[] = { 3, 4,
                   6, 8,
                   12,
                   7, 0, 5,
                   10, 3,
                   1, 2 };

  for (int i = 0; i < 12; ++i)
    add_edge(G, edges[i].first, edges[i].second, weight[i]);

  print(G, name);

  adjacency_list<listS, vecS, directedS, 
    plugin<vertex_color, default_color_type> > G_copy(N);

  cout << "Starting graph:" << endl;

  std::ostream_iterator<int> cout_int(std::cout, " ");
  std::ostream_iterator<char> cout_char(std::cout, " ");

  boost::breadth_first_search
    (G, vertex(a, G), make_bfs_visitor(
     std::make_pair(write_property(name, cout_char, on_discover_vertex()),
     std::make_pair(write_property(distance.begin(), cout_int, 
                                   on_discover_vertex()),
     std::make_pair(print_edge(name, std::cout, on_examine_edge()),
                    print_endl(std::cout, on_finish_vertex()
                    ))))));

  dijkstra_shortest_paths(G, vertex(a, G), distance.begin(),
    make_ucs_visitor(std::make_pair(copy_graph(G_copy, on_examine_edge()),
				    record_predecessors(parent.begin(), 
							on_edge_relaxed()))));

  cout << endl;
  cout << "Result:" << endl;
  breadth_first_search
    (G_copy, vertex(a, G_copy), make_bfs_visitor(
     std::make_pair(write_property(name, cout_char, on_discover_vertex()),
     std::make_pair(write_property(distance.begin(), cout_int, 
                                   on_discover_vertex()),
     std::make_pair(print_edge(name, std::cout, on_examine_edge()),
                    print_endl(std::cout, on_finish_vertex()
                    ))))));
  return 0;
}
