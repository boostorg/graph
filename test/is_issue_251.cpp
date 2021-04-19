//=======================================================================
// Copyright 2021 John Maddock.
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

//
// This file will compile ONLY if we have the conditions necessary to
// trigger: https://github.com/boostorg/graph/issues/251.
//
// This file and all references to it in Jamfile.v2 should be removed
// when that issue is fixed.
//
#include <version>

#if defined(CI_SUPPRESS_KNOWN_ISSUES) && defined(_LIBCPP_VERSION) && (_LIBCPP_VERSION >= 9000)
// We have the issue.
#else
#error "Not issue 251"
#endif
