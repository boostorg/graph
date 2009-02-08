// (C) Copyright 2007-2009 Andrew Sutton
//
// Use, modification and distribution are subject to the
// Boost Software License, Version 1.0 (See accompanying file
// LICENSE_1_0.txt or http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_UNDIRECTED_GRAPH_HPP
#define BOOST_GRAPH_UNDIRECTED_GRAPH_HPP

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

namespace boost
{
    struct undirected_graph_tag { };

    template <typename VertexProperty = no_property,
            typename EdgeProperty = no_property,
            typename GraphProperty = no_property>
class undirected_graph
{
    typedef property<vertex_index_t, unsigned, VertexProperty> vertex_property;
    typedef property<edge_index_t, unsigned, EdgeProperty> edge_property;
public:
    typedef adjacency_list<listS,
                listS,
                undirectedS,
                vertex_property,
                edge_property,
                GraphProperty,
                listS> graph_type;

private:
    // storage selectors
    typedef typename graph_type::vertex_list_selector vertex_list_selector;
    typedef typename graph_type::edge_list_selector edge_list_selector;
    typedef typename graph_type::out_edge_list_selector out_edge_list_selector;
    typedef typename graph_type::directed_selector directed_selector;

public:
    typedef undirected_graph_tag graph_tag;

    // types for properties and bundling
    typedef typename graph_type::graph_property_type graph_property_type;
    typedef typename graph_type::vertex_property_type vertex_property_type;
    typedef typename graph_type::edge_property_type edge_property_type;
    typedef typename graph_type::vertex_bundled vertex_bundled;
    typedef typename graph_type::edge_bundled edge_bundled;

    // more commonly used graph types
    typedef typename graph_type::stored_vertex stored_vertex;
    typedef typename graph_type::vertices_size_type vertices_size_type;
    typedef typename graph_type::edges_size_type edges_size_type;
    typedef typename graph_type::degree_size_type degree_size_type;
    typedef typename graph_type::vertex_descriptor vertex_descriptor;
    typedef typename graph_type::edge_descriptor edge_descriptor;

    // iterator types
    typedef typename graph_type::vertex_iterator vertex_iterator;
    typedef typename graph_type::edge_iterator edge_iterator;
    typedef typename graph_type::out_edge_iterator out_edge_iterator;
    typedef typename graph_type::in_edge_iterator in_edge_iterator;
    typedef typename graph_type::adjacency_iterator adjacency_iterator;

    // miscellaneous types
    typedef typename graph_type::directed_category directed_category;
    typedef typename graph_type::edge_parallel_category edge_parallel_category;
    typedef typename graph_type::traversal_category traversal_category;

    typedef unsigned vertex_index_type;
    typedef unsigned edge_index_type;

    inline undirected_graph(const GraphProperty& p = GraphProperty())
        : m_graph(p)
        , m_num_vertices(0)
        , m_num_edges(0)
        , m_max_vertex_index(0)
        , m_max_edge_index(0)
    { }

    inline undirected_graph(const undirected_graph& x)
        : m_graph(x)
        , m_num_vertices(x.m_num_vertices)
        , m_num_edges(x.m_num_edges)
        , m_max_vertex_index(x.m_max_vertex_index)
        , m_max_edge_index(x.m_max_edge_index)
    { }

    inline undirected_graph(vertices_size_type n,
                            const GraphProperty& p = GraphProperty())
        : m_graph(n, p)
        , m_num_vertices(n)
        , m_num_edges(0)
        , m_max_vertex_index(n)
        , m_max_edge_index(0)
    { }

    template <typename EdgeIterator>
    inline undirected_graph(EdgeIterator f,
                            EdgeIterator l,
                            vertices_size_type n,
                            edges_size_type m = 0,
                            const GraphProperty& p = GraphProperty())
        : m_graph(f, l, n, m, p)
        , m_num_vertices(n)
        , m_num_edges(0)
        , m_max_vertex_index(n)
        , m_max_edge_index(0)
    {
        // Can't always guarantee that the number of edges is actually
        // m if distance(f, l) != m (or is undefined).
        m_num_edges = m_max_edge_index = boost::num_edges(m_graph);
    }

    inline undirected_graph& operator =(const undirected_graph& g)
    {
        if(&g != this) {
            m_graph = g.m_graph;
            m_num_vertices = g.m_num_vertices;
            m_num_edges = g.m_num_edges;
            m_max_vertex_index = g.m_max_vertex_index;
        }
        return *this;
    }

    // The impl_() methods are not part of the public interface.
    inline graph_type& impl()
    { return m_graph; }

    inline const graph_type& impl() const
    { return m_graph; }


    // The following methods are not part of the public interface
    inline vertices_size_type num_vertices() const
    { return m_num_vertices; }

    inline vertex_descriptor add_vertex()
    {
        vertex_descriptor v = boost::add_vertex(m_graph);
        boost::put(vertex_index, m_graph, v, m_max_vertex_index);
        m_num_vertices++;
        m_max_vertex_index++;
        return v;
    }

    inline void clear_vertex(vertex_descriptor v)
    {
        std::pair<out_edge_iterator, out_edge_iterator>
        p = boost::out_edges(v, m_graph);
        m_num_edges -= std::distance(p.first, p.second);
        boost::clear_vertex(v, m_graph);
    }

    inline void remove_vertex(vertex_descriptor v)
    {
        boost::remove_vertex(v, m_graph);
        --m_num_vertices;
    }

    inline edges_size_type num_edges() const
    { return m_num_edges; }

    inline std::pair<edge_descriptor, bool>
    add_edge(vertex_descriptor u,
            vertex_descriptor v)
    {
        std::pair<edge_descriptor, bool> ret = boost::add_edge(u, v, m_graph);
        if(ret.second) {
            boost::put(edge_index, m_graph, ret.first, m_max_edge_index);
            ++m_num_edges;
            ++m_max_edge_index;
        }
        return ret;
    }

    inline void remove_edge(vertex_descriptor u, vertex_descriptor v)
    {
        // find all edges, (u, v)
        std::vector<edge_descriptor> edges;
        out_edge_iterator i, i_end;
        for(tie(i, i_end) = boost::out_edges(u, m_graph); i != i_end; ++i) {
            if(boost::target(*i, m_graph) == v) {
                edges.push_back(*i);
            }
        }
        // remove all edges, (u, v)
        typename std::vector<edge_descriptor>::iterator
        j = edges.begin(), j_end = edges.end();
        for( ; j != j_end; ++j) {
            remove_edge(*j);
        }
    }

    inline void remove_edge(edge_iterator i)
    {
        remove_edge(*i);
    }

    inline void remove_edge(edge_descriptor e)
    {
        boost::remove_edge(e, m_graph);
        --m_num_edges;
    }

    inline vertex_index_type max_vertex_index() const
    { return m_max_vertex_index; }

    inline void renumber_vertex_indices()
    {
        vertex_iterator i, i_end;
        tie(i, i_end) = vertices(m_graph);
        m_max_vertex_index = renumber_vertex_indices(i, i_end, 0);
    }

    inline void
    remove_vertex_and_renumber_indices(vertex_iterator i)
    {
        vertex_iterator j = next(i), end = vertices(m_graph).second;
        vertex_index_type n = get(vertex_index, m_graph, *i);

        // remove the offending vertex and renumber everything after
        remove_vertex(*i);
        m_max_vertex_index = renumber_vertex_indices(j, end, n);
    }


    inline edge_index_type max_edge_index() const
    { return m_max_edge_index; }

    inline void renumber_edge_indices()
    {
        edge_iterator i, end;
        tie(i, end) = edges(m_graph);
        m_max_edge_index = renumber_edge_indices(i, end, 0);
    }

    inline void
    remove_edge_and_renumber_indices(edge_iterator i)
    {
        edge_iterator j = next(i), end = edges(m_graph.second);
        edge_index_type n = get(edge_index, m_graph, *i);

        // remove the edge and renumber everything after it
        remove_edge(*i);
        m_max_edge_index = renumber_edge_indices(j, end, n);
    }

    inline void renumber_indices()
    {
        renumber_vertex_indices();
        renumber_edge_indices();
    }

    // bundled property support
#ifndef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
    vertex_bundled& operator [](vertex_descriptor v)
    { return m_graph[v]; }

    const vertex_bundled& operator [](vertex_descriptor v) const
    { return m_graph[v]; }

    edge_bundled& operator [](edge_descriptor e)
    { return m_graph[e]; }

    const edge_bundled& operator [](edge_descriptor e) const
    { return m_graph[e]; }
#endif

    // Graph concepts
    static inline vertex_descriptor null_vertex()
    { return graph_type::null_vertex(); }

    inline void clear()
    {
        m_graph.clear();
        m_num_vertices = m_max_vertex_index = 0;
        m_num_edges = m_max_edge_index = 0;
    }

    inline void swap(undirected_graph& g)
    {
        m_graph.swap(g);
        std::swap(m_num_vertices, g.m_num_vertices);
        std::swap(m_max_vertex_index, g.m_max_vertex_index);
        std::swap(m_num_edges, g.m_num_edges);
        std::swap(m_max_edge_index, g.m_max_edge_index);
    }

private:
    inline vertices_size_type
    renumber_vertex_indices(vertex_iterator i,
                            vertex_iterator end,
                            vertices_size_type n)
    {
        typedef typename property_map<graph_type, vertex_index_t>::type IndexMap;
        IndexMap indices = get(vertex_index, m_graph);
        for( ; i != end; ++i) {
            indices[*i] = n++;
        }
        return n;
    }

    inline edges_size_type
    renumber_edge_indices(edge_iterator i,
                            edge_iterator end,
                            edges_size_type n)
    {
        typedef typename property_map<graph_type, edge_index_t>::type IndexMap;
        IndexMap indices = get(edge_index, m_graph);
        for( ; i != end; ++i) {
            indices[*i] = n++;
        }
        return n;
    }

    graph_type m_graph;
    vertices_size_type m_num_vertices;
    edges_size_type m_num_edges;
    vertex_index_type m_max_vertex_index;
    edge_index_type m_max_edge_index;
};

// IncidenceGraph concepts
template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::vertex_descriptor
source(typename undirected_graph<VP,EP,GP>::edge_descriptor e,
    const undirected_graph<VP,EP,GP> &g)
{
    return source(e, g.impl());
}

template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::vertex_descriptor
target(typename undirected_graph<VP,EP,GP>::edge_descriptor e,
    const undirected_graph<VP,EP,GP> &g)
{
    return target(e, g.impl());
}

template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::degree_size_type
out_degree(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
        const undirected_graph<VP,EP,GP> &g)
{
    return out_degree(v, g.impl());
}

template <class VP, class EP, class GP>
inline std::pair<
    typename undirected_graph<VP,EP,GP>::out_edge_iterator,
    typename undirected_graph<VP,EP,GP>::out_edge_iterator
>
out_edges(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
        const undirected_graph<VP,EP,GP> &g)
{
    return out_edges(v, g.impl());
}

// BidirectionalGraph concepts
template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::degree_size_type
in_degree(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
        const undirected_graph<VP,EP,GP> &g)
{
    return in_degree(v, g.impl());
}

template <class VP, class EP, class GP>
inline std::pair<
    typename undirected_graph<VP,EP,GP>::in_edge_iterator,
    typename undirected_graph<VP,EP,GP>::in_edge_iterator
>
in_edges(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
        const undirected_graph<VP,EP,GP> &g)
{
    return in_edges(v, g.impl());
}


template <class VP, class EP, class GP>
inline std::pair<
    typename undirected_graph<VP,EP,GP>::out_edge_iterator,
    typename undirected_graph<VP,EP,GP>::out_edge_iterator
>
incident_edges(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                const undirected_graph<VP,EP,GP> &g)
{
    return out_edges(v, g.impl());
}

template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::degree_size_type
degree(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
    const undirected_graph<VP,EP,GP> &g)
{
    return degree(v, g.impl());
}

// AdjacencyGraph concepts
template <class VP, class EP, class GP>
inline std::pair<
    typename undirected_graph<VP,EP,GP>::adjacency_iterator,
    typename undirected_graph<VP,EP,GP>::adjacency_iterator
    >
adjacent_vertices(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                const undirected_graph<VP,EP,GP>& g)
{
    return adjacent_vertices(v, g.impl());
}

template <class VP, class EP, class GP>
typename undirected_graph<VP,EP,GP>::vertex_descriptor
vertex(typename undirected_graph<VP,EP,GP>::vertices_size_type n,
    const undirected_graph<VP,EP,GP>& g)
{
    return vertex(g.impl());
}

template <class VP, class EP, class GP>
std::pair<typename undirected_graph<VP,EP,GP>::edge_descriptor, bool>
edge(typename undirected_graph<VP,EP,GP>::vertex_descriptor u,
    typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
    const undirected_graph<VP,EP,GP>& g)
{
    return edge(u, v, g.impl());
}

// VertexListGraph concepts
template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::vertices_size_type
num_vertices(const undirected_graph<VP,EP,GP>& g)
{
    return g.num_vertices();
}

template <class VP, class EP, class GP>
inline std::pair<
    typename undirected_graph<VP,EP,GP>::vertex_iterator,
    typename undirected_graph<VP,EP,GP>::vertex_iterator
>
vertices(const undirected_graph<VP,EP,GP>& g)
{
    return vertices(g.impl());
}

// EdgeListGraph concepts
template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::edges_size_type
num_edges(const undirected_graph<VP,EP,GP>& g)
{
    return g.num_edges();
}

template <class VP, class EP, class GP>
inline std::pair<
typename undirected_graph<VP,EP,GP>::edge_iterator,
typename undirected_graph<VP,EP,GP>::edge_iterator
>
edges(const undirected_graph<VP,EP,GP>& g)
{
    return edges(g.impl());
}


// MutableGraph concepts
template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::vertex_descriptor
add_vertex(undirected_graph<VP,EP,GP> &g)
{
    return g.add_vertex();
}


template <class VP, class EP, class GP>
inline void
clear_vertex(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
undirected_graph<VP,EP,GP> &g)
{
    return g.clear_vertex(v);
}

template <class VP, class EP, class GP>
inline void
remove_vertex(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
    undirected_graph<VP,EP,GP> &g)
{
    return g.remove_vertex(v);
}



template <class VP, class EP, class GP>
inline std::pair<typename undirected_graph<VP,EP,GP>::edge_descriptor, bool>
add_edge(typename undirected_graph<VP,EP,GP>::vertex_descriptor u,
    typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
    undirected_graph<VP,EP,GP> &g)
{
    return g.add_edge(u, v);
}


template <class VP, class EP, class GP>
inline void
remove_edge(typename undirected_graph<VP,EP,GP>::vertex_descriptor u,
    typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
    undirected_graph<VP,EP,GP> &g)
{
    return g.remove_edge(u, v);
}


template <class VP, class EP, class GP>
inline void
remove_edge(typename undirected_graph<VP,EP,GP>::edge_descriptor e,
    undirected_graph<VP,EP,GP> &g)
{
    return g.remove_edge(e);
}


template <class VP, class EP, class GP>
inline void
remove_edge(typename undirected_graph<VP,EP,GP>::edge_iterator i,
    undirected_graph<VP,EP,GP> &g)
{
    return g.remove_edge(i);
}

template <class VP, class EP, class GP, class Predicate>
inline void
remove_edge_if(Predicate pred,
    undirected_graph<VP,EP,GP> &g)

{
    return remove_edge_if(pred, g.impl());
}


template <class VP, class EP, class GP, class Predicate>
inline void
remove_incident_edge_if(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                        Predicate pred,
                        undirected_graph<VP,EP,GP> &g)
{
    return remove_out_edge_if(v, pred, g.impl());
}

template <class VP, class EP, class GP, class Predicate>
inline void
remove_out_edge_if(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                Predicate pred,
                undirected_graph<VP,EP,GP> &g)
{
    return remove_out_edge_if(v, pred, g.impl());
}

template <class VP, class EP, class GP, class Predicate>
inline void
remove_in_edge_if(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                Predicate pred,
                undirected_graph<VP,EP,GP> &g)
{
    return remove_in_edge_if(v, pred, g.impl());
}

// Helper code for working with property maps
namespace detail
{
    struct undirected_graph_vertex_property_selector
    {
        template <class UndirectedGraph, class Property, class Tag>
        struct bind_
        {
            typedef typename UndirectedGraph::graph_type Graph;
            typedef property_map<Graph, Tag> PropertyMap;
            typedef typename PropertyMap::type type;
            typedef typename PropertyMap::const_type const_type;
        };
    };

    struct undirected_graph_edge_property_selector
    {
        template <class UndirectedGraph, class Property, class Tag>
        struct bind_
        {
            typedef typename UndirectedGraph::graph_type Graph;
            typedef property_map<Graph, Tag> PropertyMap;
            typedef typename PropertyMap::type type;
            typedef typename PropertyMap::const_type const_type;
        };
    };
}

template <>
struct vertex_property_selector<undirected_graph_tag>
{
    typedef detail::undirected_graph_vertex_property_selector type;
};

template <>
struct edge_property_selector<undirected_graph_tag>
{
    typedef detail::undirected_graph_edge_property_selector type;
};

// PropertyGraph concepts
template <class VP, class EP, class GP, typename Property>
inline typename property_map<undirected_graph<VP,EP,GP>, Property>::type
get(Property p, undirected_graph<VP,EP,GP>& g)
{
    return get(p, g.impl());
}

template <class VP, class EP, class GP, typename Property>
inline typename property_map<undirected_graph<VP,EP,GP>, Property>::const_type
get(Property p, const undirected_graph<VP,EP,GP>& g)
{
    return get(p, g.impl());
}

template <class VP, class EP, class GP, typename Property, typename Key>
inline typename property_traits<
    typename property_map<typename undirected_graph<VP,EP,GP>::graph_type, Property>::const_type
>::value_type
get(Property p, const undirected_graph<VP,EP,GP> &g, const Key& k)
{
    return get(p, g.impl(), k);
}

template <class VP, class EP, class GP, typename Property, typename Key, typename Value>
inline void
put(Property p, undirected_graph<VP,EP,GP> &g, const Key& k, const Value& v)
{
    put(p, g.impl(), k, v);
}

template <class VP, class EP, class GP, class Property>
typename graph_property<undirected_graph<VP,EP,GP>, Property>::type&
get_property(undirected_graph<VP,EP,GP>& g, Property p)
{
    return get_property(g.impl(), p);
}

template <class VP, class EP, class GP, class Property>
const typename graph_property<undirected_graph<VP,EP,GP>, Property>::type&
get_property(const undirected_graph<VP,EP,GP>& g, Property p)
{
    return get_property(g.impl(), p);
}

template <class VP, class EP, class GP, class Property, class Value>
void
set_property(undirected_graph<VP,EP,GP>& g, Property p, Value v)
{
    return set_property(g.impl(), p, v);
}

#ifndef BOOST_GRAPH_NO_BUNDLED_PROPERTIES
template <class VP, class EP, class GP, typename Type, typename Bundle>
inline typename property_map<undirected_graph<VP,EP,GP>, Type Bundle::*>::type
get(Type Bundle::* p, undirected_graph<VP,EP,GP>& g)
{
typedef typename property_map<undirected_graph<VP,EP,GP>, Type Bundle::*>::type
    return_type;
return return_type(&g, p);
}

template <class VP, class EP, class GP, typename Type, typename Bundle>
inline typename property_map<undirected_graph<VP,EP,GP>, Type Bundle::*>::const_type
get(Type Bundle::* p, const undirected_graph<VP,EP,GP>& g)
{
typedef typename property_map<undirected_graph<VP,EP,GP>, Type Bundle::*>::const_type
    return_type;
return return_type(&g, p);
}

template <class VP, class EP, class GP, typename Type, typename Bundle, typename Key>
inline Type
get(Type Bundle::* p, const undirected_graph<VP,EP,GP> &g, const Key& k)
{
return get(p, g.impl(), k);
}

template <class VP, class EP, class GP, typename Type, typename Bundle, typename Key, typename Value>
inline void
put(Type Bundle::* p, undirected_graph<VP,EP,GP> &g, const Key& k, const Value& v)
{
// put(get(p, g.impl()), k, v);
put(p, g.impl(), k, v);
}
#endif

// Vertex index management

template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::vertex_index_type
get_vertex_index(typename undirected_graph<VP,EP,GP>::vertex_descriptor v,
                    const undirected_graph<VP,EP,GP>& g)
{ return get(vertex_index, g, v); }

template <class VP, class EP, class GP>
typename undirected_graph<VP,EP,GP>::vertex_index_type
max_vertex_index(const undirected_graph<VP,EP,GP>& g)
{ return g.max_vertex_index(); }

template <class VP, class EP, class GP>
inline void
renumber_vertex_indices(undirected_graph<VP,EP,GP>& g)
{ g.renumber_vertex_indices(); }

template <class VP, class EP, class GP>
inline void
remove_vertex_and_renumber_indices(typename undirected_graph<VP,EP,GP>::vertex_iterator i,
                                    undirected_graph<VP,EP,GP>& g)
{ g.remove_vertex_and_renumber_indices(i); }


// Edge index management

template <class VP, class EP, class GP>
inline typename undirected_graph<VP,EP,GP>::edge_index_type
get_edge_index(typename undirected_graph<VP,EP,GP>::edge_descriptor v,
                const undirected_graph<VP,EP,GP>& g)
{ return get(edge_index, g, v); }

template <class VP, class EP, class GP>
typename undirected_graph<VP,EP,GP>::edge_index_type
max_edge_index(const undirected_graph<VP,EP,GP>& g)
{ return g.max_edge_index(); }

template <class VP, class EP, class GP>
inline void
renumber_edge_indices(undirected_graph<VP,EP,GP>& g)
{
    g.renumber_edge_indices();
}

template <class VP, class EP, class GP>
inline void
remove_edge_and_renumber_indices(typename undirected_graph<VP,EP,GP>::edge_iterator i,
                                    undirected_graph<VP,EP,GP>& g)
{ g.remove_edge_and_renumber_indices(i); }

// Index management
template <class VP, class EP, class GP>
inline void
renumber_indices(undirected_graph<VP,EP,GP>& g)
{ g.renumber_indices(); }

} /* namespace boost */

#endif
