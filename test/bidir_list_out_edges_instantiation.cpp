//=======================================================================
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Forces every member of the bidirectional helper to compile for listS out edges, as a DLL export does (issue 229).

#include <boost/graph/adjacency_list.hpp>

using EdgeProperty = boost::property< boost::edge_weight_t, double >;
using Graph = boost::adjacency_list< boost::listS, boost::listS, boost::bidirectionalS, boost::no_property, EdgeProperty >;
using Config = boost::detail::adj_list_gen< Graph, boost::listS, boost::listS, boost::bidirectionalS, boost::no_property, EdgeProperty, boost::no_property, boost::listS >::config;

template struct boost::bidirectional_graph_helper_with_property< Config >;
