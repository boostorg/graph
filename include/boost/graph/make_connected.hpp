//=======================================================================
// Copyright 2007 Aaron Windsor
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
#include <functional>
#include <vector>

#include <boost/graph/planar_detail/add_edge_visitors.hpp>
#include <boost/graph/planar_detail/bucket_sort.hpp>

namespace boost
{

// Copies the visitor. Wrap it in std::ref to have the caller's own
// visitor updated in place.
template < typename Graph, typename VertexIndexMap, typename AddEdgeVisitor >
void make_connected(Graph& g, VertexIndexMap vm, const AddEdgeVisitor& vis)
{
    AddEdgeVisitor local(vis);
    auto& vis_ref = ::boost::graph::detail::deref_visitor(local);

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
            vis_ref.visit_vertex_pair(*ci_prev, *ci, g);
    }
}

template < typename Graph, typename VertexIndexMap, typename AddEdgeVisitor >
BOOST_DEPRECATED("pass the visitor by value, wrapped in std::ref when its state is needed. Removal planned for Boost 1.95.")
::std::enable_if_t< !::boost::graph::detail::is_reference_wrapper_v< AddEdgeVisitor > > make_connected(Graph& g, VertexIndexMap vm, AddEdgeVisitor& vis)
{
    make_connected(g, vm, std::ref(vis));
}

template < typename Graph, typename VertexIndexMap >
inline void make_connected(Graph& g, VertexIndexMap vm)
{
    make_connected(g, vm, default_add_edge_visitor());
}

template < typename Graph > inline void make_connected(Graph& g)
{
    make_connected(g, get(vertex_index, g));
}

} // namespace boost

#endif //__MAKE_CONNECTED_HPP__
