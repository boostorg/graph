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
#ifndef BOOST_GRAPH_CONCEPTS_HPP
#define BOOST_GRAPH_CONCEPTS_HPP

#include <boost/config.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/concept_checks.hpp>

// where to put out_degree(v,g)?

namespace boost {

  template <class T>
  struct MultiPassInputIterator_concept {
    void constraints() {
      REQUIRE(T, InputIterator);
    }
  };

  // not sure about iterator requirements...
  // add directed_category in a couple places

  template <class G>
  struct Graph_concept
  {
    typedef typename graph_traits<G>
      ::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<G>
      ::edge_descriptor edge_descriptor;
    typedef typename graph_traits<G>::directed_category directed_category;
    typedef typename graph_traits<G>::edge_parallel_category
      edge_parallel_category;
    void constraints() {
      REQUIRE(vertex_descriptor, DefaultConstructible);
      REQUIRE(vertex_descriptor, EqualityComparable);
      REQUIRE(vertex_descriptor, Assignable);
      REQUIRE(edge_descriptor, DefaultConstructible);
      REQUIRE(edge_descriptor, EqualityComparable);
      REQUIRE(edge_descriptor, Assignable);
    }
    G g;
  };

  template <class G>
  struct IncidenceGraph_concept
  {
    typedef typename graph_traits<G>::out_edge_iterator
      out_edge_iterator;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE(out_edge_iterator, MultiPassInputIterator);

      p = out_edges(v, g);
      e = *p.first;
      u = source(e, g);
      v = target(e, g);
    }
    void const_constraints(const G& g) {
      p = out_edges(v, g);
      e = *p.first;
      u = source(e, g);
      v = target(e, g);
    }
    std::pair<out_edge_iterator, out_edge_iterator> p;
    typename graph_traits<G>::vertex_descriptor u, v;
    typename graph_traits<G>::edge_descriptor e;
    G g;
  };

  template <class G>
  struct BidirectionalGraph_concept
  {
    typedef typename graph_traits<G>::in_edge_iterator
      in_edge_iterator;
    void constraints() {
      REQUIRE(G, IncidenceGraph);
      REQUIRE(in_edge_iterator, MultiPassInputIterator);

      p = in_edges(v, g);
      e = *p.first;
      const_constraints(g);
    }
    void const_constraints(const G& g) {
      p = in_edges(v, g);
      e = *p.first;
    }
    std::pair<in_edge_iterator, in_edge_iterator> p;
    typename graph_traits<G>::vertex_descriptor v;
    typename graph_traits<G>::edge_descriptor e;
    G g;
  };

  template <class G>
  struct AdjacencyGraph_concept
  {
    typedef typename graph_traits<G>::adjacency_iterator
      adjacency_iterator;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE(adjacency_iterator, MultiPassInputIterator);

      p = adjacent_vertices(v, g);
      v = *p.first;
      const_constraints(g);
    }
    void const_constraints(const G& g) {
      p = adjacent_vertices(v, g);
    }
    std::pair<adjacency_iterator,adjacency_iterator> p;
    typename graph_traits<G>::vertex_descriptor v;
    G g;
  };

  template <class G>
  struct VertexListGraph_concept
  {
    typedef typename graph_traits<G>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<G>::vertices_size_type vertices_size_type;
    void constraints() {
      REQUIRE(G, AdjacencyGraph);
      REQUIRE(G, IncidenceGraph);
      REQUIRE(vertex_iterator, MultiPassInputIterator);

      p = vertices(g);
      v = *p.first;
      const_constraints(g);
    }
    void const_constraints(const G& g) {
      V = num_vertices(g);
      p = vertices(g);
      v = *p.first;
    }
    std::pair<vertex_iterator,vertex_iterator> p;
    typename graph_traits<G>::vertex_descriptor v;
    G g;
    vertices_size_type V;
  };

  template <class G>
  struct EdgeListGraph_concept
  {
    typedef typename graph_traits<G>::edge_iterator edge_iterator;
    typedef typename graph_traits<G>::edges_size_type edges_size_type;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE(edge_iterator, MultiPassInputIterator);

      p = edges(g);
      e = *p.first;
      u = source(e, g);
      v = target(e, g);
      const_constraints(g);
    }
    void const_constraints(const G& g) {
      p = edges(g);
      e = *p.first;
      E = num_edges(g);
      u = source(e, g);
      v = target(e, g);
    }
    std::pair<edge_iterator,edge_iterator> p;
    typename graph_traits<G>::vertex_descriptor u, v;
    typename graph_traits<G>::edge_descriptor e;
    G g;
    edges_size_type E;
  };

  template <class G>
  struct VertexAndEdgeListGraph_concept
  {
    void constraints() {
      REQUIRE(G, VertexListGraph);
      REQUIRE(G, EdgeListGraph);
    }
  };

  template <class G>
  struct MutableGraph_concept
  {
    typedef typename graph_traits<G>::edge_descriptor edge_descriptor;
    void constraints() {
      v = add_vertex(g);
      clear_vertex(v, g);
      remove_vertex(v, g);
      p = add_edge(u, v, g);
      remove_edge(u, v, g);
      remove_edge(e, g);
      remove_edge(iter, g);
    }
    G g;
    edge_descriptor e;
    std::pair<edge_descriptor, bool> p;
    typename graph_traits<G>::vertex_descriptor u, v;
    typename graph_traits<G>::out_edge_iterator iter;
  };

  template <class G>
  struct MutablePropertyGraph_concept
  {
    typedef typename graph_traits<G>::edge_descriptor edge_descriptor;
    void constraints() {
      REQUIRE(G, MutableGraph);
      v = add_vertex(vp, g);
      p = add_edge(u, v, ep, g);
    }
    G g;
    std::pair<edge_descriptor, bool> p;
    typename graph_traits<G>::vertex_descriptor u, v;
    typename graph_traits<G>::vertex_property_type vp;
    typename graph_traits<G>::edge_property_type ep;
  };

  template <class G>
  struct AdjacencyMatrix_concept
  {
    typedef typename graph_traits<G>::edge_descriptor edge_descriptor;
    void constraints() {
      REQUIRE(G, Graph);
      
      p = edge(u, v, g);
      const_constraints(g);
    }
    void const_constraints(const G& g) {
      p = edge(u, v, g);
    }
    typename graph_traits<G>::vertex_descriptor u, v;
    std::pair<edge_descriptor, bool> p;
    G g;
  };

  template <class G, class X, class Property>
  struct PropertyGraph_concept
  {
    typedef typename property_map<G, Property>::type Map;
    typedef typename property_map<G, Property>::const_type const_Map;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE2(Map, X, ReadWritePropertyMap);
      REQUIRE2(const_Map, X, ReadablePropertyMap);

      Map pmap = get(Property(), g);
      pval = get(Property(), g, x);
      put(Property(), g, x, pval);
      ignore_unused_variable_warning(pmap);
    }
    void const_constraints(const G& g) {
      const_Map pmap = get(Property(), g);
      pval = get(Property(), g, x);
      ignore_unused_variable_warning(pmap);
    }
    G g;
    X x;
    typename property_traits<Map>::value_type pval;
  };

  // This needs to move out of the graph library
  template <class B>
  struct Buffer_concept
  {
    void constraints() {
      b.push(t);
      b.pop();
      typename B::value_type& v = b.top();
      const_constraints(b);
      ignore_unused_variable_warning(v);
    }
    void const_constraints(const B& b) {
      const typename B::value_type& v = b.top();
      n = b.size();
      bool e = b.empty();
      ignore_unused_variable_warning(v);
      ignore_unused_variable_warning(e);
    }
    typename B::size_type n;
    typename B::value_type t;
    B b;
  };

  template <class C>
  struct ColorValue_concept
  {
    void constraints() {
      REQUIRE(C, EqualityComparable);

      c = white(c);
      c = gray(c);
      c = black(c);
    }
    C c;
  };

} // namespace boost

#endif /* BOOST_GRAPH_CONCEPTS_H */
