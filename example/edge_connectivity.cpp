//=======================================================================
// Copyright 2000 University of Notre Dame.
// Authors: Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee
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
#include <set>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edmunds_karp_max_flow.hpp>

using namespace boost;

template <class UndirGraph>
inline
std::pair<typename graph_traits<UndirGraph>::vertex_descriptor,
          typename graph_traits<UndirGraph>::degree_size_type>
min_degree_node(UndirGraph& g)
{
  typename graph_traits<UndirGraph>::vertex_descriptor p;
  typedef typename graph_traits<UndirGraph>::degree_size_type size_type;
  size_type delta = std::numeric_limits<size_type>::max();

  typename graph_traits<UndirGraph>::vertex_iterator i, iend;
  for (tie(i, iend) = vertices(g); i != iend; ++i)
    if (degree(*i, g) < delta) {
      delta = degree(*i, g);
      p = *i;
    }
  return std::make_pair(p, delta);
}

template <class Graph, class OutputIterator>
void neighbors(const Graph& g, 
               typename graph_traits<Graph>::vertex_descriptor u,
               OutputIterator result)
{
  typename graph_traits<Graph>::adjacency_iterator ai, aend;
  for (tie(ai, aend) = out_edges(u, g); ai != aend; ++ai)
    *result++ = *ai;
}

template <class Graph, class VertexIterator, class OutputIterator>
void neighbors(const Graph& g, 
               VertexIterator first, VertexIterator last,
               OutputIterator result)
{
  for (; first != last; ++first)
    neighbors(g, *first, result);
}

template <class UndirGraph, class OutputIterator>
typename graph_traits<UndirGraph>::degree_size_type
edge_connectivity(UndirGraph& g, OutputIterator disconnecting_set)
{
  //---------------------------------------------------------------------------
  // Type Definitions
  typedef graph_traits<UndirGraph> Traits;
  typedef typename Traits::vertex_iterator vertex_iterator;
  typedef typename Traits::edge_iterator edge_iterator;
  typedef typename Traits::out_edge_iterator out_edge_iterator;
  typedef typename Traits::vertex_descriptor vertex_descriptor;
  typedef typename Traits::degree_size_type degree_size_type;
  typedef color_traits<default_color_type> Color;

  typedef adjacency_list_traits<vecS, vecS, directedS> Tr;
  typedef adjacency_list<vecS, vecS, directedS, no_property, 
    property<edge_capacity_t, degree_size_type,
      property<edge_residual_capacity_t, degree_size_type,
        property<edge_reverse_t, typename Tr::edge_descriptor> > > > FlowGraph;
  typedef typename graph_traits<FlowGraph>::edge_descriptor edge_descriptor;

  //---------------------------------------------------------------------------
  // Variable Declarations
  vertex_descriptor u, v, p, k;
  edge_descriptor e1, e2;
  bool inserted;
  vertex_iterator vi, vi_end;
  edge_iterator ei, ei_end;
  degree_size_type delta, alpha_star, alpha_S_k;
  std::set<vertex_descriptor> S, neighbor_S;
  std::vector<vertex_descriptor> S_star, non_neighbor_S;
  std::vector<default_color_type> color(num_vertices(g));
  std::vector<edge_descriptor> pred(num_vertices(g));

  //---------------------------------------------------------------------------
  // Create a network flow graph out of the undirected graph
  FlowGraph flow_g(num_vertices(g));

  typename property_map<FlowGraph, edge_capacity_t>::type
    cap = get(edge_capacity, flow_g);
  typename property_map<FlowGraph, edge_residual_capacity_t>::type
    res_cap = get(edge_residual_capacity, flow_g);
  typename property_map<FlowGraph, edge_reverse_t>::type
    rev_edge = get(edge_reverse, flow_g);

  for (tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
    u = source(*ei, g), v = target(*ei, g);
    tie(e1, inserted) = add_edge(u, v, flow_g);
    cap[e1] = 1;
    tie(e2, inserted) = add_edge(v, u, flow_g);
    cap[e2] = 1; // not sure about this
    rev_edge[e1] = e2;
    rev_edge[e2] = e1;
  }

  //---------------------------------------------------------------------------
  // now for the algorithm
  
  tie(p, delta) = min_degree_node(g);
  S_star.push_back(p);
  alpha_star = delta;
  S.insert(p);
  neighbor_S.insert(p);
  neighbors(g, S.begin(), S.end(), 
	    std::inserter(neighbor_S, neighbor_S.begin()));
  
  std::set_difference(vertices(g).first, vertices(g).second,
                      neighbor_S.begin(), neighbor_S.end(),
                      std::back_inserter(non_neighbor_S));

  while (!non_neighbor_S.empty()) {
    k = non_neighbor_S.front();
    alpha_S_k = edmunds_karp_max_flow(flow_g, p, k, cap, res_cap, rev_edge, 
                                      &color[0], &pred[0]);
    if (alpha_S_k < alpha_star) {
      alpha_star = alpha_S_k;
      S_star.clear();
      for (tie(vi, vi_end) = vertices(flow_g); vi != vi_end; ++vi)
        if (color[*vi] != Color::white())
          S_star.push_back(*vi);
    }
    S.insert(k);
    neighbor_S.insert(k);
    neighbors(g, k, std::inserter(neighbor_S, neighbor_S.begin()));
    non_neighbor_S.clear();
    std::set_difference(vertices(g).first, vertices(g).second,
                        neighbor_S.begin(), neighbor_S.end(),
                        std::back_inserter(non_neighbor_S));
  }
  //---------------------------------------------------------------------------
  // Compute forward edges of the cut [S*, ~S*]
  std::vector<bool> in_S_star(num_vertices(g), false);
  typename std::vector<vertex_descriptor>::iterator si;
  for (si = S_star.begin(); si != S_star.end(); ++si)
    in_S_star[*si] = true;

  degree_size_type c = 0;
  for (si = S_star.begin(); si != S_star.end(); ++si) {
    out_edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = out_edges(*si, g); ei != ei_end; ++ei)
      if (!in_S_star[target(*ei, g)]) {
	*disconnecting_set++ = *ei;
	++c;
      }
  }
  return c;
}


int
main()
{
  const int N = 8;
  typedef adjacency_list<vecS, vecS, undirectedS> UndirGraph;
  UndirGraph g(N);

  add_edge(0, 1, g);
  add_edge(0, 2, g);
  add_edge(0, 3, g);
  add_edge(1, 2, g);
  add_edge(1, 3, g);
  add_edge(2, 3, g);
  add_edge(3, 4, g);
  add_edge(3, 7, g);
  add_edge(4, 5, g);
  add_edge(4, 6, g);
  add_edge(4, 7, g);
  add_edge(5, 6, g);
  add_edge(5, 7, g);
  add_edge(6, 7, g);

  typedef graph_traits<UndirGraph>::edge_descriptor edge_descriptor;
  typedef graph_traits<UndirGraph>::degree_size_type degree_size_type;
  std::vector<edge_descriptor> disconnecting_set;
  degree_size_type c = edge_connectivity(g, back_inserter(disconnecting_set));

  std::cout << "The edge connectivity is " << c << "." << std::endl;
  std::cout << "The disconnecting set is {";

  std::copy(disconnecting_set.begin(), disconnecting_set.end(), 
	    std::ostream_iterator<edge_descriptor>(std::cout, " "));
  std::cout << "}." << std::endl;
  
  return 0;
}
