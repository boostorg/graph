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

#ifndef BOOST_GRAPH_RELAX_HPP
#define BOOST_GRAPH_RELAX_HPP

#include <functional>
#include <boost/limits.hpp> // for numeric limits
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>

#include <cstdlib>  // for abs
#include <cmath>
#ifdef BOOST_NO_STDC_NAMESPACE
namespace std { using ::abs; }
#endif

namespace boost {

    template <class Graph, class WeightMap, 
            class PredecessorMap, class DistanceMap, 
            class BinaryFunctionExt, class BinaryFunctionSum>
    bool relax(typename graph_traits<Graph>::edge_descriptor e, 
               const Graph& g, WeightMap w, 
	       PredecessorMap p, DistanceMap d, 
               BinaryFunctionExt extend, BinaryFunctionSum summarize)
    {
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      Vertex u = source(e, g), v = target(e, g);
      typedef typename property_traits<DistanceMap>::value_type D;
      typedef typename property_traits<WeightMap>::value_type W;
      D d_u = get(d, u), d_v = get(d, v);
      W w_e = get(w, e);

      D d_e = extend(d_u, w_e);
      if (summarize(d_e, d_v) == d_v)
	return false;
      else if (summarize(d_e, d_v) == d_e) {
        put(d, v, d_e);
	put(p, v, u);
        return true;
      } else
	assert(false);
    }

    namespace detail {
      struct min_operation
      {
	template <class T>
	T operator()(const T& a, const T& b) const {
	  return std::min(a, b);
	}
      };
      struct closed_plus {
	template <class T>
	T operator()(const T& a, const T& b) const
	{
	  // Don't want to exceed infinity and overflow
	  // and inf should be the annihilator for +
	  T inf = std::numeric_limits<T>::max();
	  T dist_to_inf = std::abs(inf - a);
	  if (b > 0 && dist_to_inf < b)
	    return std::numeric_limits<T>::max();
	  return a + b;
	}
      };
    } // namespace detail

    template <class Graph, class WeightMap, 
      class PredecessorMap, class DistanceMap>
    bool relax(typename graph_traits<Graph>::edge_descriptor e,
               const Graph& g, WeightMap w, PredecessorMap p, DistanceMap d)
    {
      typedef typename property_traits<DistanceMap>::value_type D;
      detail::closed_plus extend;
      detail::min_operation summarize;
      return relax(e, g, w, p, d, extend, summarize);
    }

} // namespace boost

#endif /* BOOST_GRAPH_RELAX_HPP */
