//=======================================================================
// Copyright 2005 Trustees of Indiana University
// Copyright (C) 2026 Arnaud Becheler
// Authors: Andrew Lumsdaine, Douglas Gregor
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_SIMPLE_POINT_HPP
#define BOOST_GRAPH_SIMPLE_POINT_HPP

#include <boost/container_hash/hash.hpp>

#include <cmath>
#include <cstddef>
#include <type_traits>

namespace boost
{

template < typename T > struct simple_point
{
    T x;
    T y;

    // Integral coordinates widen to double so that distances are not truncated.
    using distance_type = typename std::conditional< std::is_same< T, long double >::value, long double, typename std::conditional< std::is_same< T, float >::value, float, double >::type >::type;

    friend distance_type distance(const simple_point& a, const simple_point& b)
    {
        return std::hypot(static_cast< distance_type >(a.x) - static_cast< distance_type >(b.x), static_cast< distance_type >(a.y) - static_cast< distance_type >(b.y));
    }

    constexpr friend bool operator==(simple_point const& a, simple_point const& b) noexcept
    {
        return a.x == b.x && a.y == b.y;
    }

    constexpr friend bool operator!=(simple_point const& a, simple_point const& b) noexcept
    {
        return !(a == b);
    }

    friend std::size_t hash_value(simple_point const& p)
    {
        std::size_t seed = 0;
        ::boost::hash_combine(seed, p.x);
        ::boost::hash_combine(seed, p.y);
        return seed;
    }
};

} // end namespace boost

#endif // BOOST_GRAPH_SIMPLE_POINT_HPP
