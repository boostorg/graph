// Copyright 2005 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_PYTHON_DONE_HPP
#define BOOST_GRAPH_PYTHON_DONE_HPP

namespace boost { namespace graph { namespace python {

class done
{
 public:
  virtual ~done() { }
  virtual bool operator()() const = 0;
};

} } } // end namespace boost::graph::python

#endif // BOOST_GRAPH_PYTHON_DONE_HPP
