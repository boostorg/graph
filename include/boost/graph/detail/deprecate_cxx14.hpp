//=======================================================================
// Copyright (C) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_DETAIL_DEPRECATE_CXX14_HPP
#define BOOST_GRAPH_DETAIL_DEPRECATE_CXX14_HPP

#include <boost/config.hpp>
#include <boost/config/pragma_message.hpp>

#if defined(BOOST_NO_CXX17_IF_CONSTEXPR) || defined(BOOST_NO_CXX17_INLINE_VARIABLES) \
    || defined(BOOST_NO_CXX17_DEDUCTION_GUIDES) || defined(BOOST_NO_CXX17_HDR_VARIANT) \
    || defined(BOOST_NO_CXX17_AUTO_NONTYPE_TEMPLATE_PARAMS) || defined(BOOST_NO_CXX17_HDR_OPTIONAL)
#if !defined(BOOST_GRAPH_DISABLE_DEPRECATED_14_WARNING)
BOOST_PRAGMA_MESSAGE("CAUTION: Boost.Graph in Boost 1.93 deprecates support for C++14 and will require C++17 from Boost 1.95 onwards.")
BOOST_PRAGMA_MESSAGE("CAUTION: Define BOOST_GRAPH_DISABLE_DEPRECATED_14_WARNING to suppress this message.")
#endif
#endif

#endif
