// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_POINT2D_HPP
#define BOOST_GRAPH_PYTHON_POINT2D_HPP

#include <iostream>

namespace boost { namespace graph { namespace python {
  struct point2d
  {
    double x;
    double y;
  };

  inline std::ostream& operator<<(std::ostream& out, point2d p)
  { return out << p.x << ' ' << p.y; }

  inline std::istream& operator>>(std::istream& in, point2d& p)
  { return in >> p.x >> p.y; }
} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_POINT2D_HPP
