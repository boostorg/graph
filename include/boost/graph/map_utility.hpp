/* boost/graph/map_utility.hpp header file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_GRAPH_MAP_UTILITY_HPP
#define BOOST_GRAPH_MAP_UTILITY_HPP

/*
 * Defines the boost::readable_property_map_tag struct.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::graph_traits class template.
 */
#include <boost/graph/graph_traits.hpp>

namespace boost {

/*
 * This class template encapsulates the boost::vertex() free function in the ()
 * and [] operators.  To do so, it maintains a const reference to the underlying
 * graph.
 */
template <typename VertexListGraph>
class inverse_index_map
{
 public:
    /*
     * These typedefs help this class model Adaptable Unary Functor.
     */
    typedef typename graph_traits<VertexListGraph>::vertices_size_type
            argument_type;
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor
            result_type;

    /*
     * These typedefs help this class model Readable Property Map.
     */
    typedef argument_type
            key_type;
    typedef result_type
            value_type;
    typedef value_type&
            reference;
    typedef readable_property_map_tag
            category;

 private:
    const VertexListGraph& m_g;

 public:
    explicit inverse_index_map(const VertexListGraph& g) : m_g(g)
    {
    }

    inverse_index_map(const inverse_index_map& copy) : m_g(copy.m_g)
    {
    }

    result_type operator[](const argument_type index) const
    {
        return vertex(index, m_g);
    }

    result_type operator()(const argument_type index) const
    {
        return vertex(index, m_g);
    }
};
}  // namespace boost

#endif /* BOOST_GRAPH_MAP_UTILITY_HPP */

