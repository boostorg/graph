// Copyright 2004 The Trustees of Indiana University.

// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_RANDOM_LAYOUT_HPP
#define BOOST_GRAPH_RANDOM_LAYOUT_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/type_traits/is_integral.hpp>
#include <boost/mpl/if.hpp>

namespace boost {

template<typename Graph, typename PositionMap, 
         typename RandomNumberGenerator>
void
random_graph_layout
 (const Graph& g, PositionMap position_map,
  typename property_traits<PositionMap>::value_type const& origin,
  typename property_traits<PositionMap>::value_type const& extent,
  RandomNumberGenerator& gen)
{
  typedef typename property_traits<PositionMap>::value_type Point;
  typedef double Dimension;

  typedef typename mpl::if_<is_integral<Dimension>,
                            uniform_int<Dimension>,
                            uniform_real<Dimension> >::type distrib_t;
  typedef typename mpl::if_<is_integral<Dimension>,
                            RandomNumberGenerator&,
                            uniform_01<RandomNumberGenerator, Dimension> >
    ::type gen_t;

  gen_t my_gen(gen);
  distrib_t x(origin.x, origin.x + extent.x);
  distrib_t y(origin.y, origin.y + extent.y);
  typename graph_traits<Graph>::vertex_iterator vi, vi_end;
  for(tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
    position_map[*vi].x = x(my_gen);
    position_map[*vi].y = y(my_gen);
  }
}

} // end namespace boost

#endif // BOOST_GRAPH_RANDOM_LAYOUT_HPP
