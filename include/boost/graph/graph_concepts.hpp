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
#include <boost/property_accessor.hpp>
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
      clear_vertex(g, v);
      remove_vertex(g, v);
      p = add_edge(g, u, v);
      remove_edge(g, u, v);
      //remove_edge(g, e); // needs to be implemented -JGS
    }
    G g;
    edge_descriptor e;
    std::pair<edge_descriptor, bool> p;
    typename graph_traits<G>::vertex_descriptor u, v;
  };

  template <class G>
  struct MutablePropertyGraph_concept
  {
    typedef typename graph_traits<G>::edge_descriptor edge_descriptor;
    void constraints() {
      REQUIRE(G, MutableGraph);
      v = add_vertex(g, vp);
      p = add_edge(g, u, v, ep);
    }
    G g;
    std::pair<edge_descriptor, bool> p;
    typename graph_traits<G>::vertex_descriptor u, v;
    typename graph_traits<G>::vertex_plugin_type vp;
    typename graph_traits<G>::edge_plugin_type ep;
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

  template <class G, class Tag>
  struct VertexPropertyGraph_concept
  {
    typedef typename graph_traits<G>::vertex_descriptor Vertex;
    typedef typename vertex_property_accessor<G,Tag>::type PA;
    typedef typename vertex_property_accessor<G,Tag>::const_type const_PA;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE2(PA, Vertex, ReadWritePropertyAccessor);
      REQUIRE2(const_PA, Vertex, ReadablePropertyAccessor);

      PA pa = get_vertex_property_accessor(g, Tag());
      ignore_unused_variable_warning(pa);
    }
    void const_constraints(const G& g) {
      const_PA pa = get_vertex_property_accessor(g, Tag());
      ignore_unused_variable_warning(pa);
    }
    G g;
  };

  template <class G, class Tag>
  struct EdgePropertyGraph_concept
  {
    typedef typename graph_traits<G>::edge_descriptor Edge;
    typedef typename edge_property_accessor<G,Tag>::type PA;
    typedef typename edge_property_accessor<G,Tag>::const_type const_PA;
    void constraints() {
      REQUIRE(G, Graph);
      REQUIRE2(PA, Edge, ReadWritePropertyAccessor);
      REQUIRE2(const_PA, Edge, ReadablePropertyAccessor);

      PA pa = get_edge_property_accessor(g, Tag());
    }
    void const_constraints(const G& g) {
      const_PA pa = get_edge_property_accessor(g, Tag());
    }
    G g;
  };

#if 0
  struct initialize_shortest_paths {
    void operator()(Vertex v) {
      
    }
  };

  dijkstra_shortest_paths( 
    make_visitor(
		 make_pair(initialize_shortest_paths(), 
			   initialize_vertex_tag()),
		 make_pair(predecessor_recorder(p.begin()), 
			   explore_edge_tag())
		 )
    );

  bellman_ford_shortest_paths(
    make_visitor(
		 make_pair(initialize_shortest_paths(), 
			   initialize_vertex_tag()),
		 make_pair(predecessor_recorder(p.begin()), 
			   relax_edge_tag())
		 )
   );
  /*
    Advantages:
    1. visitor functionality separate from callback event point
    2. unifies UserVisitor edge category stuff with rest of visitor
      callback events
    3. easy to extend to different algorithms. Each algorithm says
      which tags it will invoke.

    Disadvantages:
    1. more complicated for user
    2. deeper template nesting, perhaps compiler problems
  
   */


  // T is a Vertex or Edge depending on the Tag
  template <class Visitor, class T, class Graph, class Tag>
  struct Visitor_concept {
    void constraints () {
      visitor(x, g, tag);
    }
    Visitor visitor;
    Graph g;
    T x;
    Tag tag;
  };

  template <class VisitorList>
  struct controling_visitor {
    template <class EdgeOrVertex, class Graph, class Tag>
    void visit( ) {

    }
    VisitorList m_list;
  };
#endif

  template <class V, class Vertex, class Edge, class Graph>
  struct Visitor_concept
  {
    void constraints() {
      v.initialize(u);
      v.start(u);
      v.discover(u);
      bool b = v.explore(e, g);
      v.finish(u);
      ignore_unused_variable_warning(b);
    }
    V v;
    Vertex u;
    Edge e;
    Graph g;
  };

  template <class V, class Vertex, class Edge, class Graph>
  struct UserVisitor_concept
  {
#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
    typedef typename V::edge_filter_category Category;
    void constraints() {
      v.initialize(u);
      v.start(u);
      v.discover(u);
      v.explore(e, g);
      v.finish(u);
    }
    V v;
    Vertex u;
    Edge e;
    Graph g;
#else
    void constraints() { }
#endif
  };

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class V1, class V2, class Vertex, class Edge, class Graph>
  struct UserVisitor_concept< std::pair<V1,V2>, Vertex,Edge,Graph>
  {
    void constraints() {
      REQUIRE4(V1, Vertex, Edge, Graph, UserVisitor);
      REQUIRE4(V2, Vertex, Edge, Graph, UserVisitor);
    }
  };
#endif

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
