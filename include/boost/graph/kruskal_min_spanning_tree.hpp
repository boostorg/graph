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
#ifndef BOOST_GRAPH_MST_KRUSKAL_HPP
#define BOOST_GRAPH_MST_KRUSKAL_HPP

/*
 *Minimum Spanning Tree 
 *         Kruskal Algorithm
 *
 *Requirement:
 *      undirected graph
 */

#include <vector>
#include <queue>
#include <functional>

#include <boost/property_map.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/pending/indirect_cmp.hpp>


namespace boost {

  // Kruskal's algorithm for Minimum Spanning Tree
  //
  // This is a greedy algorithm to calculate the Minimum Spanning Tree
  // for an undirected graph with weighted edges. The output will be a
  // set of edges.
  //


  // Variant (1)
  template <class Graph, class OutputIterator, 
            class Rank, class Parent>
  inline void 
  kruskal_minimum_spanning_tree(Graph& G, OutputIterator spanning_tree_edges, 
                                Rank rank, Parent parent)
  {
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    kruskal_minimum_spanning_tree(G, spanning_tree_edges, rank, parent, 
                                  get(edge_weight, G));
  }

  // Variant (2)
  template <class Graph, class OutputIterator, 
            class Rank, class Parent, class Weight>
  void
  kruskal_minimum_spanning_tree(Graph& G, OutputIterator spanning_tree_edges, 
                                Rank rank, Parent parent, Weight weight)
  {
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename graph_traits<Graph>::edge_descriptor Edge;
    function_requires<VertexAndEdgeListGraphConcept<Graph> >();
    function_requires<OutputIteratorConcept<OutputIterator, Edge> >();
    function_requires<ReadWritePropertyMapConcept<Rank, Vertex> >();
    function_requires<ReadWritePropertyMapConcept<Parent, Vertex> >();
    function_requires<ReadablePropertyMapConcept<Weight, Edge> >();
    typedef typename property_traits<Weight>::value_type W_value;
    typedef typename property_traits<Rank>::value_type R_value;
    typedef typename property_traits<Parent>::value_type P_value;
    function_requires<LessThanComparableConcept<W_value> >();
    function_requires<ConvertibleConcept<P_value, Vertex> >();
    function_requires<IntegerConcept<R_value> >();

    disjoint_sets<Rank, Parent>  dset(rank, parent);
    
    typename graph_traits<Graph>::vertex_iterator ui, uiend;
    for (boost::tie(ui, uiend) = vertices(G); ui != uiend; ++ui)
      dset.make_set(*ui);

    typedef typename property_traits<Weight>::value_type W;
    typedef indirect_cmp<Weight, std::greater<W> > weight_greater;

    weight_greater wl(weight);

    typedef std::priority_queue <Edge, std::vector<Edge>, weight_greater>
      Queue;
    Queue Q(wl);

    /*push all edge into Q*/
    typename graph_traits<Graph>::edge_iterator ei, eiend;
    for (boost::tie(ei,eiend) = edges(G); ei != eiend; ++ei) 
      Q.push(*ei);
    
    while ( !Q.empty() ) {
      Edge e = Q.top();
      Q.pop();
      Vertex u = dset.find_set(source(e, G));
      Vertex v = dset.find_set(target(e, G));
      if ( u != v ) {
        *spanning_tree_edges++ = e;
        dset.link(u, v);
      }
    }
  }

} /*namespace*/


#endif /*BOOST_GRAPH_MST_KRUSKAL_H*/

