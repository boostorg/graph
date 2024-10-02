//=======================================================================
// Copyright 2024
// Author: Hermann Stamm-Wilbrandt
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_UNDIRECTED_GRAPH_CONSTANT_TIME_EDGE_ADD_AND_REMOVE.HPP
#define BOOST_GRAPH_UNDIRECTED_GRAPH_CONSTANT_TIME_EDGE_ADD_AND_REMOVE.HPP

#include <utility>
#include <list>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/undirected_graph.hpp>

enum edge_oeip_t
{
    edge_oeip
};

namespace boost
{
BOOST_INSTALL_PROPERTY(edge, oeip);


/**
 * The undirected_graph_constant_time_edge_add_and_remove class
 * extends undirected_graph class template of the BGL. 
 * Its only purpose over undirected_graph is to provide 
 * O(1) time remove_edge() (and add_edge()) functions.
 */
#define GRAPH_TYPE \
    undirected_graph< VP, property< edge_oeip_t, void*, EP >, GP >

template< class VP, class EP, class GP>
class undirected_graph_constant_time_edge_add_and_remove
    : public GRAPH_TYPE
{
 public:
    typedef GRAPH_TYPE graph_type;

    typedef typename graph_type::vertices_size_type vertices_size_type;
    typedef typename graph_type::vertex_descriptor vertex_descriptor;
    typedef typename graph_type::edge_descriptor edge_descriptor;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    typedef typename graph_type::edge_iterator edge_iterator;

 public:
    inline undirected_graph_constant_time_edge_add_and_remove()
        : graph_type()
        , m_map(get(edge_oeip, *this))
    {
    }

    // would miss necessary linkage between x and created graph
    inline undirected_graph_constant_time_edge_add_and_remove(
        graph_type const& x) = delete;

    // would miss necessary linkage between graphs
    inline undirected_graph_constant_time_edge_add_and_remove(
        vertices_size_type n, GP const& p = GP()) = delete;

    template < typename EdgeIterator >
    inline undirected_graph_constant_time_edge_add_and_remove(EdgeIterator f,
        EdgeIterator l,
        typename GRAPH_TYPE::vertices_size_type n,
        typename GRAPH_TYPE::edges_size_type m = 0,
        GP const& p = GP()) = delete;

    // too slow
    std::pair< edge_descriptor, bool > add_edge(
        vertex_descriptor u, vertex_descriptor v, EP const& p) = delete;

    // O(1)
    inline std::pair< edge_descriptor, bool >
        add_edge(vertex_descriptor u, vertex_descriptor v)
    {
        std::pair< edge_descriptor, bool > et = graph_type::add_edge(u, v);
        BOOST_ASSERT(et.second);

        storage.push_back(std::make_pair(--out_edges(u, *this).second,
                                         --out_edges(v, *this).second));
        m_map[et.first] = reinterpret_cast<void*>(&storage.back());
        return et;
    }

    // too slow
    void remove_edge(vertex_descriptor u, vertex_descriptor v) = delete;

    // O(1)
    void remove_edge(edge_iterator i) { remove_edge(*i); }

    // O(1)
    inline void remove_edge(edge_descriptor e)
    {
        graph_type::remove_edge(m_map[e]);
    }

    // O(degree(v))
    inline void clear_vertex(vertex_descriptor v)
    {
        while (true)
        {
            out_edge_iterator ei, ei_end;
            boost::tie(ei, ei_end) = out_edges(v, *this);
            if (ei == ei_end)
                break;
            this->remove_edge(*ei);
        }
    }

 private:
    typename property_map< graph_type, edge_oeip_t >::type m_map;
    std::list< std::pair< out_edge_iterator, out_edge_iterator > > storage;
};

} /* namespace boost */


#endif
