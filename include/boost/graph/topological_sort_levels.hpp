//
//=======================================================================
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_TOPOLOGICAL_SORT_LEVELS_HPP
#define BOOST_GRAPH_TOPOLOGICAL_SORT_LEVELS_HPP

#include <vector>
#include <boost/config.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/exception.hpp>
#include <boost/throw_exception.hpp>

namespace boost
{

// Topological Sort into Levels
//
// Like topological_sort, but groups vertices by "level" rather than producing
// a single linear ordering. Level 0 contains every vertex with no incoming
// edges; level k > 0 contains every vertex whose longest incoming path comes
// from a vertex at level k - 1.
//
// Edge convention follows the rest of BGL: an edge (u, v) means u must
// precede v, so level(u) < level(v). Level numbering runs forward, from
// sources at level 0 to sinks at the highest level.
//
// Implemented via Kahn's algorithm. Same concept requirements as
// topological_sort: VertexListGraph + IncidenceGraph.

namespace detail
{

    template < typename VertexListGraph, typename LevelMap,
        typename VertexIndexMap >
    typename graph_traits< VertexListGraph >::vertices_size_type
    topological_sort_levels_impl(
        const VertexListGraph& g, LevelMap level, VertexIndexMap index_map)
    {
        typedef typename graph_traits< VertexListGraph >::vertex_descriptor
            Vertex;
        typedef typename graph_traits< VertexListGraph >::vertices_size_type
            size_type;
        typedef typename graph_traits< VertexListGraph >::vertex_iterator
            vertex_iter;
        typedef typename graph_traits< VertexListGraph >::out_edge_iterator
            out_edge_iter;

        const size_type n = num_vertices(g);
        std::vector< size_type > in_degree(n, 0);

        vertex_iter vi, vi_end;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        {
            out_edge_iter ei, ei_end;
            for (boost::tie(ei, ei_end) = out_edges(*vi, g); ei != ei_end;
                 ++ei)
            {
                ++in_degree[get(index_map, target(*ei, g))];
            }
        }

        std::vector< Vertex > current_level;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        {
            if (in_degree[get(index_map, *vi)] == 0)
            {
                current_level.push_back(*vi);
                put(level, *vi, size_type(0));
            }
        }

        size_type total_emitted = current_level.size();
        size_type level_count = current_level.empty() ? size_type(0)
                                                      : size_type(1);

        std::vector< Vertex > next_level;
        while (!current_level.empty())
        {
            next_level.clear();
            for (typename std::vector< Vertex >::const_iterator it
                 = current_level.begin();
                 it != current_level.end(); ++it)
            {
                out_edge_iter ei, ei_end;
                for (boost::tie(ei, ei_end) = out_edges(*it, g); ei != ei_end;
                     ++ei)
                {
                    Vertex v = target(*ei, g);
                    size_type vidx = get(index_map, v);
                    if (--in_degree[vidx] == 0)
                    {
                        next_level.push_back(v);
                        put(level, v, level_count);
                    }
                }
            }
            if (!next_level.empty())
            {
                ++level_count;
                total_emitted += next_level.size();
            }
            current_level.swap(next_level);
        }

        if (total_emitted != n)
        {
            BOOST_THROW_EXCEPTION(not_a_dag());
        }

        return level_count;
    }

    template < typename VertexListGraph, typename VertexIndexMap >
    void topological_sort_levels_to_buckets(const VertexListGraph& g,
        VertexIndexMap index_map,
        std::vector< std::vector< typename graph_traits<
            VertexListGraph >::vertex_descriptor > >& levels)
    {
        typedef typename graph_traits< VertexListGraph >::vertices_size_type
            size_type;

        std::vector< size_type > level_of(num_vertices(g), size_type(0));

        const size_type num_levels = topological_sort_levels_impl(g,
            make_iterator_property_map(level_of.begin(), index_map),
            index_map);

        levels.clear();
        levels.resize(num_levels);

        typename graph_traits< VertexListGraph >::vertex_iterator vi, vi_end;
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        {
            levels[level_of[get(index_map, *vi)]].push_back(*vi);
        }
    }

} // namespace detail

// Property-map form. Writes level[v] = k for every vertex v and returns the
// number of levels. LevelMap must be a writable property map keyed on the
// graph's vertex descriptor with a value type convertible from
// vertices_size_type.
template < typename VertexListGraph, typename LevelMap, typename P, typename T,
    typename R >
typename graph_traits< VertexListGraph >::vertices_size_type
topological_sort_levels(const VertexListGraph& g, LevelMap level,
    const bgl_named_params< P, T, R >& params)
{
    return detail::topological_sort_levels_impl(g, level,
        choose_const_pmap(get_param(params, vertex_index), g, vertex_index));
}

template < typename VertexListGraph, typename LevelMap >
typename graph_traits< VertexListGraph >::vertices_size_type
topological_sort_levels(const VertexListGraph& g, LevelMap level)
{
    return topological_sort_levels(
        g, level, bgl_named_params< int, buffer_param_t >(0));
}

// Convenience form. Resizes <tt>levels</tt> to hold one inner vector per
// level; on return, <tt>levels[k]</tt> contains every vertex assigned to
// level k.
template < typename VertexListGraph, typename P, typename T, typename R >
void topological_sort_levels(const VertexListGraph& g,
    std::vector< std::vector< typename graph_traits<
        VertexListGraph >::vertex_descriptor > >& levels,
    const bgl_named_params< P, T, R >& params)
{
    detail::topological_sort_levels_to_buckets(g,
        choose_const_pmap(get_param(params, vertex_index), g, vertex_index),
        levels);
}

template < typename VertexListGraph >
void topological_sort_levels(const VertexListGraph& g,
    std::vector< std::vector< typename graph_traits<
        VertexListGraph >::vertex_descriptor > >& levels)
{
    topological_sort_levels(
        g, levels, bgl_named_params< int, buffer_param_t >(0));
}

} // namespace boost

#endif // BOOST_GRAPH_TOPOLOGICAL_SORT_LEVELS_HPP
