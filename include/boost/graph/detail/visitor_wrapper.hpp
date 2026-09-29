//=======================================================================
// Copyright (C) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_DETAIL_VISITOR_WRAPPER_HPP
#define BOOST_GRAPH_DETAIL_VISITOR_WRAPPER_HPP

#include <functional>
#include <type_traits>

namespace boost
{
namespace graph
{
    namespace detail
    {

        // true when the visitor was passed with std::ref
        template < class Visitor >
        struct is_reference_wrapper : std::false_type
        {
        };

        template < class Visitor >
        struct is_reference_wrapper< std::reference_wrapper< Visitor > >
        : std::true_type
        {
        };

        // the visitor type the caller wrote, for concept checks
        template < class Visitor > struct unwrap_visitor
        {
            using type = Visitor;
        };

        template < class Visitor >
        struct unwrap_visitor< std::reference_wrapper< Visitor > >
        {
            using type = Visitor;
        };

        template < class Visitor >
        struct unwrap_visitor< const std::reference_wrapper< Visitor > >
        {
            using type = Visitor;
        };

        // resolve a std::ref wrapped visitor to the referenced object
        template < class Visitor > Visitor& deref_visitor(Visitor& vis)
        {
            return vis;
        }

        template < class Visitor >
        Visitor& deref_visitor(std::reference_wrapper< Visitor > vis)
        {
            return vis.get();
        }

    } // namespace detail
} // namespace graph
} // namespace boost

#endif // BOOST_GRAPH_DETAIL_VISITOR_WRAPPER_HPP
