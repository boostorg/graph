// Copyright 2004-5 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_GENERATORS_HPP
#define BOOST_GRAPH_PYTHON_GENERATORS_HPP

#include <cstdlib>

namespace boost { namespace graph { namespace python {

struct erdos_renyi
{
  erdos_renyi(std::size_t n, double p) : n(n), p(p) { }

  std::size_t n;
  double p;
};

struct power_law_out_degree
{
  power_law_out_degree(std::size_t n, double alpha, double beta)
    : n(n), alpha(alpha), beta(beta) { }

  std::size_t n;
  double alpha;
  double beta;
};

struct small_world
{
  small_world(std::size_t n, std::size_t k, double p) : n(n), k(k), p(p) { }

  std::size_t n;
  std::size_t k;
  double p;
};

} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_GENERATORS_HPP
