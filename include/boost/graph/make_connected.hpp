//=======================================================================
// Copyright 2007 Aaron Windsor
// Copyright 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef __MAKE_CONNECTED_HPP__
#define __MAKE_CONNECTED_HPP__

#include <boost/config.hpp>
#include <boost/next_prior.hpp>
#include <boost/tuple/tuple.hpp> //for tie
#include <boost/graph/connected_components.hpp>
#include <boost/graph/detail/visitor_wrapper.hpp>
#include <boost/property_map/property_map.hpp>
#include <vector>

#include <boost/graph/planar_detail/add_edge_visitors.hpp>
#include <boost/graph/planar_detail/bucket_sort.hpp>

namespace boost
{

namespace graph_detail
{

template < typename Graph, typename VertexIndexMap, typename AddEdgeVisitor >
void make_connected_impl(Graph& g, VertexIndexMap vm, AddEdgeVisitor& vis)
{
    typedef typename graph_traits< Graph >::vertex_iterator vertex_iterator_t;
    typedef typename graph_traits< Graph >::vertex_descriptor vertex_t;
    typedef typename graph_traits< Graph >::vertices_size_type v_size_t;
    typedef iterator_property_map< typename std::vector< v_size_t >::iterator,
        VertexIndexMap >
        vertex_to_v_size_map_t;

    std::vector< v_size_t > component_vector(num_vertices(g));
    vertex_to_v_size_map_t component(component_vector.begin(), vm);
    std::vector< vertex_t > vertices_by_component(num_vertices(g));

    v_size_t num_components = connected_components(g, component);

    if (num_components < 2)
        return;

    vertex_iterator_t vi, vi_end;
    boost::tie(vi, vi_end) = vertices(g);
    std::copy(vi, vi_end, vertices_by_component.begin());

    bucket_sort(vertices_by_component.begin(), vertices_by_component.end(),
        component, num_components);

    typedef typename std::vector< vertex_t >::iterator vec_of_vertices_itr_t;

    vec_of_vertices_itr_t ci_end = vertices_by_component.end();
    vec_of_vertices_itr_t ci_prev = vertices_by_component.begin();
    if (ci_prev == ci_end)
        return;

    for (vec_of_vertices_itr_t ci = boost::next(ci_prev); ci != ci_end;
         ci_prev = ci, ++ci)
    {
        if (component[*ci_prev] != component[*ci])
            vis.visit_vertex_pair(*ci_prev, *ci, g);
    }
}

} // namespace graph_detail

namespace graph
{

// Hidden from argument dependent lookup so unqualified calls keep finding the deprecated boost:: overloads.
namespace planar_adl_barrier
{
}

using namespace planar_adl_barrier;

namespace planar_adl_barrier
{

// Copies the visitor. Wrap it in std::ref to have the caller's own
// visitor updated in place.
template < typename Graph, typename VertexIndexMap, typename AddEdgeVisitor >
void make_connected(Graph& g, VertexIndexMap vm, AddEdgeVisitor vis)
{
    auto& vis_ref = ::boost::graph::detail::deref_visitor(vis);
    ::boost::graph_detail::make_connected_impl(g, vm, vis_ref);
}

template < typename Graph, typename VertexIndexMap >
void make_connected(Graph& g, VertexIndexMap vm)
{
    ::boost::graph::make_connected(g, vm, ::boost::default_add_edge_visitor());
}

template < typename Graph > void make_connected(Graph& g)
{
    ::boost::graph::make_connected(g, get(::boost::vertex_index, g));
}

} // namespace planar_adl_barrier

} // namespace graph

template < typename Graph, typename VertexIndexMap, typename AddEdgeVisitor >
BOOST_DEPRECATED("moved to boost::graph::make_connected, which takes the visitor by value. Removal planned for Boost 1.95.")
void make_connected(Graph& g, VertexIndexMap vm, AddEdgeVisitor& vis)
{
    ::boost::graph_detail::make_connected_impl(g, vm, vis);
}

template < typename Graph, typename VertexIndexMap >
BOOST_DEPRECATED("moved to boost::graph::make_connected, which takes the visitor by value. Removal planned for Boost 1.95.")
inline void make_connected(Graph& g, VertexIndexMap vm)
{
    ::boost::graph::make_connected(g, vm);
}

template < typename Graph >
BOOST_DEPRECATED("moved to boost::graph::make_connected, which takes the visitor by value. Removal planned for Boost 1.95.")
inline void make_connected(Graph& g)
{
    ::boost::graph::make_connected(g);
}

} // namespace boost

#endif //__MAKE_CONNECTED_HPP__
