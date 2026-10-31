//=======================================================================
// Copyright 2007 Aaron Windsor
// Copyright 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef __MAKE_BICONNECTED_PLANAR_HPP__
#define __MAKE_BICONNECTED_PLANAR_HPP__

#include <boost/config.hpp>
#include <boost/tuple/tuple.hpp> //for tie
#include <boost/graph/biconnected_components.hpp>
#include <boost/property_map/property_map.hpp>
#include <vector>
#include <iterator>
#include <algorithm>

#include <boost/graph/detail/visitor_wrapper.hpp>
#include <boost/graph/planar_detail/add_edge_visitors.hpp>

namespace boost
{

namespace graph_detail
{

template < typename Graph, typename PlanarEmbedding, typename EdgeIndexMap,
    typename AddEdgeVisitor >
void make_biconnected_planar_impl(Graph& g, PlanarEmbedding embedding, EdgeIndexMap em, AddEdgeVisitor& vis)
{
    typedef typename graph_traits< Graph >::vertex_descriptor vertex_t;
    typedef typename graph_traits< Graph >::edge_descriptor edge_t;
    typedef typename graph_traits< Graph >::edges_size_type edge_size_t;
    typedef typename property_traits< PlanarEmbedding >::value_type
        embedding_value_t;
    typedef typename embedding_value_t::const_iterator embedding_iterator_t;
    typedef iterator_property_map< std::vector< std::size_t >::iterator,
        EdgeIndexMap >
        component_map_t;

    edge_size_t n_edges(num_edges(g));
    std::vector< vertex_t > articulation_points;
    std::vector< edge_size_t > component_vector(n_edges);
    component_map_t component_map(component_vector.begin(), em);

    biconnected_components(
        g, component_map, std::back_inserter(articulation_points));

    typename std::vector< vertex_t >::iterator ap, ap_end;
    ap_end = articulation_points.end();
    for (ap = articulation_points.begin(); ap != ap_end; ++ap)
    {
        vertex_t v(*ap);
        embedding_iterator_t pi = embedding[v].begin();
        embedding_iterator_t pi_end = embedding[v].end();
        edge_size_t previous_component(n_edges + 1);
        vertex_t previous_vertex = graph_traits< Graph >::null_vertex();

        for (; pi != pi_end; ++pi)
        {
            edge_t e(*pi);
            vertex_t e_source(source(e, g));
            vertex_t e_target(target(e, g));

            // Skip self-loops and parallel edges
            if (e_source == e_target || previous_vertex == e_target)
                continue;

            vertex_t current_vertex = e_source == v ? e_target : e_source;
            edge_size_t current_component = component_map[e];
            if (previous_vertex != graph_traits< Graph >::null_vertex()
                && current_component != previous_component)
            {
                vis.visit_vertex_pair(current_vertex, previous_vertex, g);
            }
            previous_vertex = current_vertex;
            previous_component = current_component;
        }
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
template < typename Graph, typename PlanarEmbedding, typename EdgeIndexMap, typename AddEdgeVisitor >
void make_biconnected_planar(Graph& g, PlanarEmbedding embedding, EdgeIndexMap em, AddEdgeVisitor vis)
{
    auto& vis_ref = ::boost::graph::detail::deref_visitor(vis);
    ::boost::graph_detail::make_biconnected_planar_impl(g, embedding, em, vis_ref);
}

template < typename Graph, typename PlanarEmbedding, typename EdgeIndexMap >
void make_biconnected_planar(Graph& g, PlanarEmbedding embedding, EdgeIndexMap em)
{
    ::boost::graph::make_biconnected_planar(g, embedding, em, ::boost::default_add_edge_visitor());
}

template < typename Graph, typename PlanarEmbedding >
void make_biconnected_planar(Graph& g, PlanarEmbedding embedding)
{
    ::boost::graph::make_biconnected_planar(g, embedding, get(::boost::edge_index, g));
}

} // namespace planar_adl_barrier

} // namespace graph

template < typename Graph, typename PlanarEmbedding, typename EdgeIndexMap, typename AddEdgeVisitor >
BOOST_DEPRECATED("moved to boost::graph::make_biconnected_planar, which takes the visitor by value. Removal planned for Boost 1.95.")
void make_biconnected_planar(Graph& g, PlanarEmbedding embedding, EdgeIndexMap em, AddEdgeVisitor& vis)
{
    ::boost::graph_detail::make_biconnected_planar_impl(g, embedding, em, vis);
}

template < typename Graph, typename PlanarEmbedding, typename EdgeIndexMap >
BOOST_DEPRECATED("moved to boost::graph::make_biconnected_planar, which takes the visitor by value. Removal planned for Boost 1.95.")
inline void make_biconnected_planar(Graph& g, PlanarEmbedding embedding, EdgeIndexMap em)
{
    ::boost::graph::make_biconnected_planar(g, embedding, em);
}

template < typename Graph, typename PlanarEmbedding >
BOOST_DEPRECATED("moved to boost::graph::make_biconnected_planar, which takes the visitor by value. Removal planned for Boost 1.95.")
inline void make_biconnected_planar(Graph& g, PlanarEmbedding embedding)
{
    ::boost::graph::make_biconnected_planar(g, embedding);
}

} // namespace boost

#endif //__MAKE_BICONNECTED_PLANAR_HPP__
