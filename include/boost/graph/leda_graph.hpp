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
#ifndef BOOST_GRAPH_LEDA_HPP
#define BOOST_GRAPH_LEDA_HPP

#include <LEDA/graph.h>

#include <boost/config.hpp>
#include <boost/pending/iterator_adaptors.hpp>

// The functions and classes in this file allows the user to
// treat a LEDA GRAPH object as a boost graph "as is". No
// wrapper is needed for the GRAPH object.

// Warning: this implementation relies on partial specialization
// for the graph_traits class (so it won't compile with Visual C++)

// Warning: this implementation is in alpha and has not been tested

namespace boost {
  
  struct out_edge_iterator_policies
  {
    static void increment(edge& e)
    { e = Succ_Adj_Edge(e,0); }

    static void decrement(edge& e)
    { e = Pred_Adj_Edge(e,0); }

    template <class Reference>
    static Reference dereference(boost::type<Reference>, const edge& e)
    { return const_cast<Reference>(e); }

    static bool equal(const edge& x, const edge& y)
    { return x == y; }
  };

  struct in_edge_iterator_policies
  {
    static void increment(edge& e)
    { e = Succ_Adj_Edge(e,1); }

    static void decrement(edge& e)
    { e = Pred_Adj_Edge(e,1); }

    template <class Reference>
    static Reference dereference(boost::type<Reference>, const edge& e)
    { return const_cast<Reference>(e); }

    static bool equal(const edge& x, const edge& y)
    { return x == y; }
  };

  struct adjacency_iterator_policies
  {
    static void increment(edge& e)
    { e = Succ_Adj_Edge(e,0); }

    static void decrement(edge& e)
    { e = Pred_Adj_Edge(e,0); }

    template <class Reference>
    static node dereference(boost::type<Reference>, const edge& e)
    { return ::target(e); }

    static bool equal(const edge& x, const edge& y)
    { return x == y; }
  };

  template <class LedaGraph>
  struct vertex_iterator_policies
  {
    vertex_iterator_policies() { }
    vertex_iterator_policies(const LedaGraph* g) : m_g(g) { }

    void increment(node& v) const
    { v = m_g->succ_node(v); }

    void decrement(node& v) const
    { v = m_g->pred_node(v); }

    template <class Reference>
    Reference dereference(boost::type<Reference>, const node& v) const
    { return const_cast<Reference>(v); }

    static bool equal(const node& x, const node& y)
    { return x == y; }

    const LedaGraph* m_g;
  };

} // namespace boost

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
namespace boost {
  template <class vtype, class etype>
  struct graph_traits< GRAPH<vtype,etype> > {
    typedef node vertex_descriptor;
    typedef edge edge_descriptor;

    typedef boost::iterator_adaptor<edge,
      boost::adjacency_iterator_policies, 
      boost::iterator<std::bidirectional_iterator_tag,
        node, std::ptrdiff_t, node*, node>
    > adjacency_iterator;

    typedef boost::iterator_adaptor<edge,
      boost::out_edge_iterator_policies, 
      boost::iterator<std::bidirectional_iterator_tag,edge>
    > out_edge_iterator;

    typedef boost::iterator_adaptor<edge,
      boost::in_edge_iterator_policies, 
      boost::iterator<std::bidirectional_iterator_tag,edge>
    > in_edge_iterator;

    typedef boost::iterator_adaptor<node,
      boost::vertex_iterator_policies< GRAPH<vtype,etype> >, 
      boost::iterator<std::bidirectional_iterator_tag,node>
    > vertex_iterator;

    typedef directed_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category; // not sure here
    typedef int size_type;
  };
} // namespace boost
#endif

namespace boost {

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor
  source(typename graph_traits< GRAPH<vtype,etype> >::edge_descriptor e,
	 const GRAPH<vtype,etype>& g)
  {
    return source(e);
  }

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor
  target(typename graph_traits< GRAPH<vtype,etype> >::edge_descriptor e,
	 const GRAPH<vtype,etype>& g)
  {
    return target(e);
  }

  template <class vtype, class etype>
  inline std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::vertex_iterator,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_iterator >  
  vertices(const GRAPH<vtype,etype>& g)
  {
    typedef typename graph_traits< GRAPH<vtype,etype> >::vertex_iterator
      Iter;
    return std::make_pair( Iter(g.first_node(),&g), Iter(0,&g) );
  }

  // no edges(g) function

  template <class vtype, class etype>
  inline std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::out_edge_iterator,
    typename graph_traits< GRAPH<vtype,etype> >::out_edge_iterator >  
  out_edges(
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, 
    const GRAPH<vtype,etype>& g)
  {
    typedef typename graph_traits< GRAPH<vtype,etype> >
      ::out_edge_iterator Iter;
    return std::make_pair( Iter(First_Adj_Edge(u,0)), Iter(0) );
  }

  template <class vtype, class etype>
  inline std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::in_edge_iterator,
    typename graph_traits< GRAPH<vtype,etype> >::in_edge_iterator >  
  in_edges(
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, 
    const GRAPH<vtype,etype>& g)
  {
    typedef typename graph_traits< GRAPH<vtype,etype> >
      ::in_edge_iterator Iter;
    return std::make_pair( Iter(First_Adj_Edge(u,1)), Iter(0) );
  }

  template <class vtype, class etype>
  inline std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::adjacency_iterator,
    typename graph_traits< GRAPH<vtype,etype> >::adjacency_iterator >  
  adjacent_vertices(typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, const GRAPH<vtype,etype>& g)
  {
    typedef typename graph_traits< GRAPH<vtype,etype> >
      ::adjacency_iterator Iter;
    return std::make_pair( Iter(First_Adj_Edge(u,0)), Iter(0) );
  }

  // deprecated
  template <class vtype, class etype>
  inline std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::adjacency_iterator,
    typename graph_traits< GRAPH<vtype,etype> >::adjacency_iterator >  
  adj(typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u,
      const GRAPH<vtype,etype>& g)
  {
    return adjacent_vertices(u, g);
  }

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::size_type
  num_vertices(const GRAPH<vtype,etype>& g)
  {
    return g.number_of_nodes();
  }  

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::size_type
  num_edges(const GRAPH<vtype,etype>& g)
  {
    return g.number_of_edges();
  }  

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::size_type
  out_degree(
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, 
    const GRAPH<vtype,etype>&)
  {
    return outdeg(u);
  }

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::size_type
  in_degree(
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, 
    const GRAPH<vtype,etype>&)
  {
    return indeg(u);
  }

  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::size_type
  degree(
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u, 
    const GRAPH<vtype,etype>&)
  {
    return outdeg(u) + indeg(u);
  }
  
  template <class vtype, class etype>
  typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor
  add_vertex(GRAPH<vtype,etype>& g)
  {
    return g.new_node();
  }

  // Hmm, LEDA doesn't have the equivalent of clear_vertex() -JGS
  // need to write an implementation
  template <class vtype, class etype>
  void clear_vertex(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u)
  {
    g.del_node(u);
  }

  template <class vtype, class etype>
  void remove_vertex(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u)
  {
    g.del_node(u);
  }

  template <class vtype, class etype>
  std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::edge_descriptor,
    bool>
  add_edge(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor v)
  {
    return std::make_pair(g.new_edge(u, v), true);
  }

  template <class vtype, class etype>
  std::pair<
    typename graph_traits< GRAPH<vtype,etype> >::edge_descriptor,
    bool>
  add_edge(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor v,
    const etype& et)
  {
    return std::make_pair(g.new_edge(u, v, et), true);
  }

  template <class vtype, class etype>
  void
  remove_edge(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor u,
    typename graph_traits< GRAPH<vtype,etype> >::vertex_descriptor v)
  {
    typename graph_traits< GRAPH<vtype,etype> >::out_edge_iterator 
      i,iend;
    for (boost::tie(i,iend) = out_edges(u,g); i != iend; ++i)
      if (target(*i,g) == v)
	g.del_edge(*i);
  }

  template <class vtype, class etype>
  void
  remove_edge(GRAPH<vtype,etype>& g,
    typename graph_traits< GRAPH<vtype,etype> >::edge_descriptor e)
  {
    g.del_edge(e);
  }
  // property maps...
  
  
} // namespace boost


#endif // BOOST_GRAPH_LEDA_HPP
