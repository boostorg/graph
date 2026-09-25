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

#include <boost/container_hash/hash.hpp>

#include <cmath>

namespace boost
{

template < typename T > struct simple_point
{
    T x;
    T y;

    // Deduce return type: float for float, double for double/ints, long double
    // for long double, to avoid silent and dangerous truncation of floating to int in distance
    using distance_type =
        typename std::conditional< std::is_same< T, long double >::value,
            long double,
            typename std::conditional< std::is_same< T, float >::value, float,
                double >::type >::type;

    constexpr friend distance_type distance(
        const simple_point& a, const simple_point& b)
    {
        return std::hypot(static_cast< distance_type >(a.x)
                - static_cast< distance_type >(b.x),
            static_cast< distance_type >(a.y)
                - static_cast< distance_type >(b.y));
    }

    constexpr friend
    bool operator==(simple_point const &a, simple_point const &b) noexcept
    {
        return a.x == b.x && a.y == b.y;
    }

    constexpr friend
    bool operator!=(simple_point const &a, simple_point const &b) noexcept
    {
        return !(a == b);
    }

    friend constexpr
    std::size_t hash_value(simple_point const& p)
    {
        std::size_t seed = 0;

        // Normalize zero values to avoid -0.0 and +0.0 hash collisions
        T x_norm = p.x == T(0) ? T(0) : p.x;
        T y_norm = p.y == T(0) ? T(0) : p.y;

        boost::hash_combine(seed, x_norm);
        boost::hash_combine(seed, y_norm);

        return seed;
    }
};


} // end namespace boost

#endif // BOOST_GRAPH_SIMPLE_POINT_HPP
