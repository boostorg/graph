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
#ifndef BOOST_GRAPH_MST_PRIM_HPP
#define BOOST_GRAPH_MST_PRIM_HPP

#include <functional>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/uniform_cost_search.hpp>

namespace boost {
  
  namespace detail {
    // this should be somewhere else in boost...
    template <class U, class V> struct _project2nd {
      V operator()(U, V v) const { return v; }
    };
  }

  // This is Prim's algorithm to calculate the Minimum Spanning Tree
  // for an undirected graph with weighted edges.

  // Variant (1)
  template <class Graph, class Vertex>
  inline void
  prim_minimum_spanning_tree(Graph& G, Vertex s)
  {
    prim_minimum_spanning_tree(G, s, 
                    get_vertex_property_accessor(G, vertex_distance()));
  }

  // Variant (2)
  template <class Graph, class Vertex, class Distance>
  inline void
  prim_minimum_spanning_tree(Graph& G, Vertex s, Distance d)
  {
    prim_minimum_spanning_tree(G, s, d, ucs_visitor<>());
  }

  // Variant (3)
  template <class Graph, class Vertex, class Distance, 
            class UniformCostVisitor>
  inline void
  prim_minimum_spanning_tree(Graph& G, Vertex s, Distance d, 
                             UniformCostVisitor visit)
  {
    prim_minimum_spanning_tree(G, s, d, 
                               get_edge_property_accessor(G, edge_weight()), 
                               get_vertex_property_accessor(G, vertex_color()), 
                               get_vertex_property_accessor(G, vertex_index()), 
                               visit);
  }

  // Variant (4)
  template <class Graph, class Vertex, class UniformCostVisitor, 
            class Distance, class Weight, class Color, class ID>
  inline void
  prim_minimum_spanning_tree(Graph& G, Vertex s,
                             Distance distance, Weight weight, Color color, 
                             ID id, UniformCostVisitor visit)
  {
    typedef typename property_traits<Distance>::value_type D;
    typedef typename property_traits<Weight>::value_type W;
    std::less<D> compare;
    detail::_project2nd<W,W> combine;

    typename boost::graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (tie(ui, ui_end) = vertices(G); ui != ui_end; ++ui)
      put(distance, *ui, std::numeric_limits<D>::max());

    put(distance, s, D());
    uniform_cost_search(G, s, distance, weight, color, id, 
                        compare, combine, visit);    
  }

} /*namespace*/

#endif /*BOOST_GRAPH_MST_PRIM_H*/
