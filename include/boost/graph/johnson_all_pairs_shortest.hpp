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
#ifndef BOOST_GRAPH_JOHNSON_HPP
#define BOOST_GRAPH_JOHNSON_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/bellman_ford_shortest_paths.hpp>
#include <boost/graph/dijkstra_shortest_paths.hpp>

namespace boost {

  template <class VertexAndEdgeListGraph, class DistanceMatrix,
            class Distance, class Weight, class Color, class VertexID>
  bool
  johnson_all_pairs_shortest_paths(VertexAndEdgeListGraph& g, 
                                   DistanceMatrix& D,
                                   Distance d, Distance h,
                                   Weight w, Color c, VertexID id)
  {
    typedef graph_traits<VertexAndEdgeListGraph> Traits;
    typename Traits::vertex_iterator v, v_end, u, u_end;
    typename Traits::edge_iterator e, e_end;
    typename Traits::vertex_descriptor s = *vertices(g).first;
    typedef typename property_traits<Distance>::value_type DT;

    put(d, s, DT()); // initialize distance of source vertex to zero
    if (bellman_ford_shortest_paths(g, num_vertices(g), w, d)) {
      for (tie(v, v_end) = vertices(g); v != v_end; ++v)
        put(h, *v, get(d,*v));
      for (tie(e, e_end) = edges(g); e != e_end; ++e)
        put(w, *e, get(w,*e) + get(h,source(*e,g)) - get(h, target(*e,g)));
      for (tie(u, u_end) = vertices(g); u != u_end; ++u) {
        dijkstra_shortest_paths(g, *u, d, w, c, id, ucs_visitor<>());
        for (tie(v, v_end) = vertices(g); v != v_end; ++v)
          D[*u][*v] = get(d, *v) + get(h, *v) - get(h, *u);
      }
      return true;
    } else
      return false;
  }

} // namespace boost

#endif // BOOST_GRAPH_JOHNSON_HPP


