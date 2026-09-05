//=======================================================================
// Copyright 2005 Trustees of Indiana University
// Authors: Andrew Lumsdaine, Douglas Gregor
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_SIMPLE_POINT_HPP
#define BOOST_GRAPH_SIMPLE_POINT_HPP

#include <cmath>

namespace boost
{

template < typename T > struct simple_point
{
    T x;
    T y;
};

// Euclidean distance between two simple_point<T> using std::hypot
template <typename T>
inline T distance(const simple_point<T>& a, const simple_point<T>& b)
{
    return std::hypot(a.x - b.x, a.y - b.y);
}

} // end namespace boost

#endif // BOOST_GRAPH_SIMPLE_POINT_HPP
