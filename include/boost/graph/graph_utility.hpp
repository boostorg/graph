//
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
//
#ifndef BOOST_GRAPH_UTILITY_HPP
#define BOOST_GRAPH_UTILITY_HPP

#include <stdlib.h>
#include <iosfwd>
#include <assert.h>
#include <boost/config.hpp>
#include <boost/utility.hpp>
#ifndef BOOST_NO_SLIST
#  include <slist> // shouldn't have to include this... -JGS
#endif
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/container_traits.hpp>

namespace boost {

  template <class Graph>
  inline 
  std::pair<typename graph_traits<Graph>::vertex_descriptor,
            typename graph_traits<Graph>::vertex_descriptor>
  incident(typename graph_traits<Graph>::edge_descriptor e,
           Graph& g)
  {
    return std::make_pair(source(e,g), target(e,g));
  }

  template <class Graph>
  inline typename graph_traits<Graph>::vertex_descriptor
  opposite(typename graph_traits<Graph>::edge_descriptor e,
	   typename graph_traits<Graph>::vertex_descriptor v,
	   const Graph& g)
  {
    if (v == source(e, g))
      return target(e, g);
    else if (v == target(e, g))
      return source(e, g);
    else
      return Edge(); // ?
  }

  // Need to convert all of these printing functions to take an ostream object
  // -JGS

  template <class IncidenceGraph, class Name>
  void print_in_edges(const IncidenceGraph& G, Name name)
  {
    typename graph_traits<IncidenceGraph>::vertex_iterator ui,ui_end;
    for (boost::tie(ui,ui_end) = vertices(G); ui != ui_end; ++ui) {
      std::cout << get(name,*ui) << " <-- ";
      typename graph_traits<IncidenceGraph>
        ::in_edge_iterator ei, ei_end;
      for(boost::tie(ei,ei_end) = in_edges(*ui,G); ei != ei_end; ++ei)
	std::cout << get(name,source(*ei,G)) << " ";
      std::cout << std::endl;
    }
  }

  template <class IncidenceGraph, class Name>
  void print_graph_dispatch(const IncidenceGraph& G, Name name, directed_tag)
  {
    typename graph_traits<IncidenceGraph>::vertex_iterator ui,ui_end;
    for (boost::tie(ui,ui_end) = vertices(G); ui != ui_end; ++ui) {
      std::cout << get(name,*ui) << " --> ";
      typename graph_traits<IncidenceGraph>
        ::out_edge_iterator ei, ei_end;
      for(boost::tie(ei,ei_end) = out_edges(*ui,G); ei != ei_end; ++ei)
	std::cout << get(name,target(*ei,G)) << " ";
      std::cout << std::endl;
    }
  }
  template <class IncidenceGraph, class Name>
  void print_graph_dispatch(const IncidenceGraph& G, Name name, undirected_tag)
  {
    typename graph_traits<IncidenceGraph>::vertex_iterator ui,ui_end;
    for (boost::tie(ui,ui_end) = vertices(G); ui != ui_end; ++ui) {
      std::cout << get(name,*ui) << " <--> ";
      typename graph_traits<IncidenceGraph>
        ::out_edge_iterator ei, ei_end;
      for(boost::tie(ei,ei_end) = out_edges(*ui,G); ei != ei_end; ++ei)
	std::cout << get(name,target(*ei,G)) << " ";
      std::cout << std::endl;
    }
  }
  template <class IncidenceGraph, class Name>
  void print_graph(const IncidenceGraph& G, Name name)
  {
    typedef typename graph_traits<IncidenceGraph>
      ::directed_category Cat;
    print_graph_dispatch(G, name, Cat());
  }
  template <class IncidenceGraph>
  void print_graph(const IncidenceGraph& G) {
    print_graph(G, get_vertex_property_accessor(G, vertex_index()));
  }

  template <class EdgeListGraph, class Name>
  void print_edges(const EdgeListGraph& G, Name name)
  {
    typename graph_traits<EdgeListGraph>::edge_iterator ei,ei_end;
    for (boost::tie(ei,ei_end) = edges(G); ei != ei_end; ++ei)
      std::cout << "(" << get(name,source(*ei,G))
		<< "," << get(name,target(*ei,G)) << ") ";
    std::cout << std::endl;
  }

  template <class EdgeListGraph, class VertexName, class EdgeName>
  void print_edges2(const EdgeListGraph& G, VertexName vname, EdgeName ename)
  {
    typename graph_traits<EdgeListGraph>::edge_iterator ei,ei_end;
    for (boost::tie(ei,ei_end) = edges(G); ei != ei_end; ++ei)
      std::cout << get(ename,*ei) << "(" << get(vname,source(*ei,G))
		<< "," << get(vname,target(*ei,G)) << ") ";
    std::cout << std::endl;
  }

  template <class VertexListGraph, class Name>
  void print_vertices(const VertexListGraph& G, Name name)
  {
    typename graph_traits<VertexListGraph>::vertex_iterator vi,vi_end;
    for (boost::tie(vi,vi_end) = vertices(G); vi != vi_end; ++vi)
      std::cout << get(name,*vi) << " ";
    std::cout << std::endl;
  }

  template <class _ForwardIter, class _Tp>
  void iota(_ForwardIter __first, _ForwardIter __last, _Tp __value)
  {
    while (__first != __last)
      *__first++ = __value++;
  }

  inline std::size_t random_number(std::size_t N) {
    std::size_t ret = rand() % N; 
    return ret;
  }

  // grab a random vertex from the graph's vertex set
  template <class Graph>
  typename graph_traits<Graph>::vertex_descriptor
  random_vertex(Graph& g)
  {
    std::size_t n = random_number(num_vertices(g));
    typename graph_traits<Graph>::vertex_iterator
      i = vertices(g).first;
    while (n-- > 0) ++i; // std::advance not VC++ portable
    return *i;
  }

  template <class Graph>
  typename graph_traits<Graph>::edge_descriptor
  random_edge(Graph& g) {
    typename Graph::size_type E = num_edges(g), n;
    n = random_number(E);
    typename graph_traits<Graph>::edge_iterator
      i = edges(g).first;
    while (n-- > 0) ++i; // std::advance not VC++ portable
    return *i;
  }

  template <class Graph, class Vertex>
  bool is_adj_dispatch(Graph& g, Vertex a, Vertex b, bidirectional_tag)
  {
    typedef typename graph_traits<Graph>::edge_descriptor 
      edge_descriptor;
    typename graph_traits<Graph>::adjacency_iterator vi, viend, 
      adj_found;
    boost::tie(vi, viend) = adjacent_vertices(a, g);
    adj_found = std::find(vi, viend, b);
    if (adj_found == viend)
      return false;  

    typename graph_traits<Graph>::out_edge_iterator oi, oiend, 
      out_found;
    boost::tie(oi, oiend) = out_edges(a, g);
    out_found = std::find(oi, oiend, edge_descriptor(a,b));
    if (out_found == oiend)
      return false;

    typename graph_traits<Graph>::in_edge_iterator ii, iiend, 
      in_found;
    boost::tie(ii, iiend) = in_edges(b, g);
    in_found = std::find(ii, iiend, edge_descriptor(a,b));
    if (in_found == iiend)
      return false;

    return true;
  }
  template <class Graph, class Vertex>
  bool is_adj_dispatch(Graph& g, Vertex a, Vertex b, directed_tag)
  {
    typedef typename graph_traits<Graph>::edge_descriptor 
      edge_descriptor;
    typename Graph::adjacency_iterator vi, viend, found;
    boost::tie(vi, viend) = adjacenc_vertices(a, g);
    found = std::find(vi, viend, b);
    if ( found == viend )
      return false;  

    typename graph_traits<Graph>::out_edge_iterator oi, oiend, 
      out_found;
    boost::tie(oi, oiend) = out_edges(a, g);
    out_found = std::find(oi, oiend, edge_descriptor(a,b));
    if (out_found == oiend)
      return false;

    return true;
  }
  template <class Graph, class Vertex>
  bool is_adj_dispatch(Graph& g, Vertex a, Vertex b, undirected_tag)
  {
    return is_adj_dispatch(g, a, b, directed_tag());
  }

  template <class Graph, class Vertex>
  bool is_adjacent(Graph& g, Vertex a, Vertex b) {
    typedef typename graph_traits<Graph>::directed_category Cat;
    return is_adj_dispatch(g, a, b, Cat());
  }

  template <class Graph, class Edge>
  bool in_edge_set(Graph& g, Edge e)
  {
    typename Graph::edge_iterator ei, ei_end, found;
    boost::tie(ei, ei_end) = edges(g);
    found = std::find(ei, ei_end, e);
    return found != ei_end;
  }

  template <class Graph, class Vertex>
  bool in_vertex_set(Graph& g, Vertex v)
  {
    typename Graph::vertex_iterator vi, vi_end, found;
    boost::tie(vi, vi_end) = vertices(g);
    found = std::find(vi, vi_end, v);
    return found != vi_end;
  }

  template <class Graph, class Vertex>
  bool in_edge_set(Graph& g, Vertex u, Vertex v)
  {
    typename Graph::edge_iterator ei, ei_end;
    for (boost::tie(ei,ei_end) = edges(g); ei != ei_end; ++ei)
      if (source(*ei,g) == u && target(*ei,g) == v)
	return true;
    return false;
  }


} /* namespace boost */

#endif /* BOOST_GRAPH_UTILITY_HPP*/
