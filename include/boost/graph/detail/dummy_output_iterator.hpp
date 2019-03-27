// Copyright (c) Jeremy Siek 2001
// Copyright (c) Douglas Gregor 2004
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_DETAIL_DUMMY_OUTPUT_ITERATOR_HPP
#define BOOST_GRAPH_DETAIL_DUMMY_OUTPUT_ITERATOR_HPP

#include <iterator>

namespace boost { namespace graph_detail {

    struct dummy_output_iterator
    {
        typedef std::output_iterator_tag iterator_category;
        typedef void value_type;
        typedef void pointer;
        typedef void difference_type;

        struct reference
        {
            template <typename T>
            reference& operator=(const T&) { return *this; }
        };

        reference operator*() const { return reference(); }
        dummy_output_iterator& operator++() { return *this; }
        dummy_output_iterator operator++(int) { return *this; }
    };
}}

#endif  // include guard

