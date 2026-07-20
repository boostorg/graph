// Copyright (C) 2009 Andrew Sutton
//
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_MUTABILITY_TRAITS_HPP
#define BOOST_GRAPH_MUTABILITY_TRAITS_HPP

#include <boost/config.hpp>
#include <type_traits>
#include <boost/type_traits/is_convertible.hpp>
#include <boost/type_traits/is_same.hpp>

namespace boost
{

// The mutabiltiy categories classify graphs by their mutating operations
// on the edge and vertex sets. This is a substantially more refined
// categorization than the MutableGraph and MutablePropertyGraph denote.
// Currently, this framework is only used in the graph tests to help
// dispatch test to the correct places. However, there are probably some
// constructive or destructive algorithms (i.e., graph generators) that
// may use these to describe requirements on graph inputs.

struct add_vertex_tag
{
};
struct add_vertex_property_tag : virtual add_vertex_tag
{
};
struct add_edge_tag
{
};
struct add_edge_property_tag : virtual add_edge_tag
{
};
struct remove_vertex_tag
{
};
struct remove_edge_tag
{
};

struct mutable_vertex_graph_tag : virtual add_vertex_tag,
                                  virtual remove_vertex_tag
{
};
struct mutable_vertex_property_graph_tag : virtual add_vertex_property_tag,
                                           virtual remove_vertex_tag
{
};

struct mutable_edge_graph_tag : virtual add_edge_tag, virtual remove_edge_tag
{
};
struct mutable_edge_property_graph_tag : virtual add_edge_property_tag,
                                         virtual remove_edge_tag
{
};

struct mutable_graph_tag : virtual mutable_vertex_graph_tag,
                           virtual mutable_edge_graph_tag
{
};
struct mutable_property_graph_tag : virtual mutable_vertex_property_graph_tag,
                                    virtual mutable_edge_property_graph_tag
{
};

// Some graphs just don't like to be torn down. Note this only restricts
// teardown to the set of vertices, not the vertex set.
// TODO: Find a better name for this tag.
struct add_only_property_graph_tag : virtual add_vertex_property_tag,
                                     virtual mutable_edge_property_graph_tag
{
};

/**
 * The graph_mutability_traits provide methods for determining the
 * interfaces supported by graph classes for adding and removing vertices
 * and edges.
 */
template < typename Graph > struct graph_mutability_traits
{
    typedef typename Graph::mutability_category category;
};

template < typename Graph >
struct graph_has_add_vertex
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          add_vertex_tag >::value >
{
};

template < typename Graph >
struct graph_has_add_vertex_with_property
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          add_vertex_property_tag >::value >
{
};

template < typename Graph >
struct graph_has_remove_vertex
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          remove_vertex_tag >::value >
{
};

template < typename Graph >
struct graph_has_add_edge
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          add_edge_tag >::value >
{
};

template < typename Graph >
struct graph_has_add_edge_with_property
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          add_edge_property_tag >::value >
{
};

template < typename Graph >
struct graph_has_remove_edge
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          remove_edge_tag >::value >
{
};

template < typename Graph >
struct is_mutable_vertex_graph
: std::integral_constant< bool,
      graph_has_add_vertex< Graph >::value
          && graph_has_remove_vertex< Graph >::value >
{
};

template < typename Graph >
struct is_mutable_vertex_property_graph
: std::integral_constant< bool,
      graph_has_add_vertex_with_property< Graph >::value
          && graph_has_remove_vertex< Graph >::value >
{
};

template < typename Graph >
struct is_mutable_edge_graph
: std::integral_constant< bool,
      graph_has_add_edge< Graph >::value
          && graph_has_remove_edge< Graph >::value >
{
};

template < typename Graph >
struct is_mutable_edge_property_graph
: std::integral_constant< bool,
      graph_has_add_edge_with_property< Graph >::value
          && graph_has_remove_edge< Graph >::value >
{
};

template < typename Graph >
struct is_mutable_graph
: std::integral_constant< bool,
      is_mutable_vertex_graph< Graph >::value
          && is_mutable_edge_graph< Graph >::value >
{
};

template < typename Graph >
struct is_mutable_property_graph
: std::integral_constant< bool,
      is_mutable_vertex_property_graph< Graph >::value
          && is_mutable_edge_property_graph< Graph >::value >
{
};

template < typename Graph >
struct is_add_only_property_graph
: std::integral_constant< bool,
      is_convertible< typename graph_mutability_traits< Graph >::category,
          add_only_property_graph_tag >::value >
{
};

/** @name Mutability Traits Specializations */
//@{

//@}

} // namespace boost

#endif
