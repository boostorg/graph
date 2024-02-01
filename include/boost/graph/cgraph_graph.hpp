/*
 * Copyright (C) 2023 Tobias Lorenz
 *
 * Author: tobias.lorenz@gmx.net
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef BOOST_GRAPH_CGRAPH_GRAPH_HPP
#define BOOST_GRAPH_CGRAPH_GRAPH_HPP

/* C++ Standard Library */
#include <cassert>

/* Boost Graph Library */
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

/* Graphviz */
#include <graphviz/cgraph.h>

namespace boost {

// @todo subgraphs are not handled yet

typedef ::Agraph_t * cgraph_graph_ptr;

#define cgraph_type Agdirected
//#define cgraph_type Agstrictdirected
//#define cgraph_type Agundirected
//#define cgraph_type Agstrictundirected

template <>
struct graph_traits<cgraph_graph_ptr>
{
    /* Graph */

    typedef ::Agnode_t * vertex_descriptor;
    typedef ::Agedge_t * edge_descriptor;

// @todo How can this runtime setting be handled in the template?
#if cgraph_type == Agdirected
    // Agdirected (digraph)
    typedef bidirectional_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
#elif cgraph_type == Agstrictdirected
    // Agstrictdirected (strict digraph)
    typedef bidirectional_tag directed_category;
    typedef disallow_parallel_edge_tag edge_parallel_category;
#elif cgraph_type == Agundirected
    // Agundirected (graph)
    typedef undirected_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
#elif cgraph_type == Agstrictundirected
    // Agstrictundirected (strict graph)
    typedef undirected_tag directed_category;
    typedef disallow_parallel_edge_tag edge_parallel_category;
#endif
    struct traversal_category:
        //public virtual incidence_graph_tag, // included in bidirectional_graph_tag
        public virtual adjacency_graph_tag,
        public virtual bidirectional_graph_tag,
        public virtual vertex_list_graph_tag,
        public virtual edge_list_graph_tag
        //public virtual adjacency_matrix_tag, => edge(u, v, g)
    {
    };

    static constexpr vertex_descriptor null_vertex()
    {
        return nullptr;
    }

    /* IncidenceGraph */

    class out_edge_iterator:
        public iterator_facade<out_edge_iterator, edge_descriptor, forward_traversal_tag, edge_descriptor>
    {
    public:
        explicit out_edge_iterator(edge_descriptor edge = nullptr, cgraph_graph_ptr graph = nullptr):
            edge(edge),
            graph(graph)
        {
        }

    private:
        reference dereference() const
        {
            return edge;
        }

        bool equal(const out_edge_iterator & j) const
        {
            return edge == j.edge;
        }

        void increment()
        {
            assert(graph);
            assert(edge);
            edge = ::agnxtout(graph, edge);
        }

        edge_descriptor edge{nullptr};
        cgraph_graph_ptr graph{nullptr};

        friend class iterator_core_access;
    };

    typedef int degree_size_type;

    /* BidirectionalGraph */

    class in_edge_iterator:
        public iterator_facade<in_edge_iterator, edge_descriptor, forward_traversal_tag, edge_descriptor>
    {
    public:
        explicit in_edge_iterator(edge_descriptor edge = nullptr, cgraph_graph_ptr graph = nullptr):
            edge(edge),
            graph(graph)
        {
        }

    private:
        reference dereference() const
        {
            return edge;
        }

        bool equal(const in_edge_iterator & j) const
        {
            return edge == j.edge;
        }

        void increment()
        {
            assert(graph);
            assert(edge);
            edge = ::agnxtin(graph, edge);
        }

        edge_descriptor edge{nullptr};
        cgraph_graph_ptr graph{nullptr};

        friend class iterator_core_access;
    };

    /* AdjacencyGraph */

    class adjacency_iterator:
        public iterator_facade<adjacency_iterator, vertex_descriptor, forward_traversal_tag, vertex_descriptor>
    {
    public:
        explicit adjacency_iterator(vertex_descriptor node = nullptr, edge_descriptor edge = nullptr, cgraph_graph_ptr graph = nullptr):
            node(node),
            edge(edge),
            graph(graph)
        {
        }

    private:
        reference dereference() const
        {
            assert(edge);
            return aghead(edge);
        }

        bool equal(const adjacency_iterator & j) const
        {
            return (node == j.node) && (edge == j.edge);
        }

        void increment()
        {
            assert(graph);
            if (::agisdirected(graph)) {
                assert(edge);
                edge = ::agnxtout(graph, edge);
            }
            if (::agisundirected(graph)) {
                assert(edge);
                assert(node);
                edge = ::agnxtedge(graph, edge, node);
            }
        }

        vertex_descriptor node{nullptr};
        edge_descriptor edge{nullptr};
        cgraph_graph_ptr graph{nullptr};

        friend class iterator_core_access;
    };

    /* VertexListGraph */

    class vertex_iterator:
        public iterator_facade<vertex_iterator, vertex_descriptor, bidirectional_traversal_tag, vertex_descriptor>
    {
    public:
        explicit vertex_iterator(vertex_descriptor node = nullptr, cgraph_graph_ptr graph = nullptr):
            node(node),
            graph(graph)
        {
        }

    private:
        reference dereference() const
        {
            return node;
        }

        bool equal(const vertex_iterator & j) const
        {
            return node == j.node;
        }

        void increment()
        {
            assert(graph);
            assert(node);
            node = ::agnxtnode(graph, node);
        }

        void decrement()
        {
            assert(graph);
            assert(node);
            node = ::agprvnode(graph, node);
        }

//        void advance(difference_type n)
//        {
//            while (n > 0) {
//                increment();
//                n--;
//            }
//            while (n < 0) {
//                decrement();
//                n++;
//            }
//        }

//        difference_type distance_to(const vertex_iterator & j) const
//        {
//            difference_type n = 0;
//            vertex_descriptor jnode = j.node;
//            while(node != jnode) {
//                jnode = ::agnxtnode(graph, jnode);
//                n++;
//            }
//            return n;
//        }

        vertex_descriptor node{nullptr};
        cgraph_graph_ptr graph{nullptr};

        friend class iterator_core_access;
    };

    typedef int vertices_size_type;

    /* EdgeListGraph */

    class edge_iterator:
        public iterator_facade<edge_iterator, edge_descriptor, forward_traversal_tag, edge_descriptor>
    {
    public:
        explicit edge_iterator(vertex_descriptor node = nullptr, edge_descriptor edge = nullptr, const cgraph_graph_ptr graph = nullptr):
            node(node),
            edge(edge),
            graph(graph)
        {
        }

    private:
        const edge_descriptor & dereference() const
        {
            return edge;
        }

        bool equal(const edge_iterator & j) const
        {
            return edge == j.edge;
        }

        void increment()
        {
            assert(graph);
            if (::agisdirected(graph)) {
                assert(edge);
                edge = ::agnxtout(graph, edge);
                if (!edge) {
                    node = ::agnxtnode(graph, node);
                    if (node) {
                        edge = ::agfstout(graph, node);
                    }
                }
            }
            if (::agisundirected(graph)) {
                assert(edge);
                assert(node);
                edge = ::agnxtedge(graph, edge, node);
                if (!edge) {
                    node = ::agnxtnode(graph, node);
                    if (node) {
                        edge = ::agfstedge(graph, node);
                    }
                }
            }
        }

        vertex_descriptor node{nullptr};
        edge_descriptor edge{nullptr};
        cgraph_graph_ptr graph{nullptr};

        friend class iterator_core_access;
    };

    typedef int edges_size_type;

    /* MutablePropertyGraph */

    // @todo the following typedefs are suffient to set the name, however std::map would allow all attributes to be set.
    typedef char * edge_property_type;
    typedef char * vertex_property_type;
};

/* IncidenceGraph */

graph_traits<cgraph_graph_ptr>::vertex_descriptor source(graph_traits<cgraph_graph_ptr>::edge_descriptor e, const cgraph_graph_ptr & g)
{
    assert(e);
    assert(g);
    return agtail(e);
}

graph_traits<cgraph_graph_ptr>::vertex_descriptor target(graph_traits<cgraph_graph_ptr>::edge_descriptor e, const cgraph_graph_ptr & g)
{
    assert(e);
    assert(g);
    return aghead(e);
}

inline std::pair<graph_traits<cgraph_graph_ptr>::out_edge_iterator, graph_traits<cgraph_graph_ptr>::out_edge_iterator> out_edges(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, const cgraph_graph_ptr & g)
{
    assert(v);
    assert(g);
    typedef graph_traits<cgraph_graph_ptr>::out_edge_iterator Iter;
    return std::make_pair(Iter(::agfstout(g, v), g), Iter(nullptr, g));
}

graph_traits<cgraph_graph_ptr>::degree_size_type out_degree(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, const cgraph_graph_ptr & g)
{
    assert(v);
    assert(g);
    // @todo agdegree or agcountuniqedges ?
    return ::agdegree(g, v, 0, 1);
}

/* BidirectionalGraph */

inline std::pair<graph_traits<cgraph_graph_ptr>::in_edge_iterator, graph_traits<cgraph_graph_ptr>::in_edge_iterator> in_edges(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, const cgraph_graph_ptr & g)
{
    assert(v);
    assert(g);
    typedef graph_traits<cgraph_graph_ptr>::in_edge_iterator Iter;
    return std::make_pair(Iter(::agfstin(g, v), g), Iter(nullptr, g));
}

graph_traits<cgraph_graph_ptr>::degree_size_type in_degree(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, const cgraph_graph_ptr & g)
{
    assert(v);
    assert(g);
    return ::agdegree(g, v, 1, 0);
}

graph_traits<cgraph_graph_ptr>::degree_size_type degree(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, const cgraph_graph_ptr & g)
{
    assert(u);
    assert(g);
    return ::agdegree(g, u, 1, 1);
}

/* AdjacencyGraph */

inline std::pair<graph_traits<cgraph_graph_ptr>::adjacency_iterator, graph_traits<cgraph_graph_ptr>::adjacency_iterator> adjacent_vertices(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, const cgraph_graph_ptr & g)
{
    assert(v);
    assert(g);
    typedef graph_traits<cgraph_graph_ptr>::adjacency_iterator Iter;
    if (::agisdirected(g)) {
        return std::make_pair(Iter(v, ::agfstout(g, v), g), Iter(v, nullptr, g));
    }
    if (::agisundirected(g)) {
        return std::make_pair(Iter(v, ::agfstedge(g, v), g), Iter(v, nullptr, g));
    }
    return std::make_pair(Iter(v, nullptr, g), Iter(v, nullptr, g));
}

/* VertexListGraph */

inline std::pair<graph_traits<cgraph_graph_ptr>::vertex_iterator, graph_traits<cgraph_graph_ptr>::vertex_iterator> vertices(const cgraph_graph_ptr & g)
{
    assert(g);
    typedef graph_traits<cgraph_graph_ptr>::vertex_iterator Iter;
    return std::make_pair(Iter(::agfstnode(g), g), Iter(nullptr, g));
}

graph_traits<cgraph_graph_ptr>::vertices_size_type num_vertices(const cgraph_graph_ptr & g)
{
    assert(g);
    return ::agnnodes(g);
}

/* EdgeListGraph */

inline std::pair<graph_traits<cgraph_graph_ptr>::edge_iterator, graph_traits<cgraph_graph_ptr>::edge_iterator> edges(const cgraph_graph_ptr & g)
{
    assert(g);
    typedef graph_traits<cgraph_graph_ptr>::edge_iterator Iter;
    graph_traits<cgraph_graph_ptr>::vertex_descriptor n = ::agfstnode(g);
    assert(n);
    if (::agisdirected(g)) {
        graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstedge(g, n);
        return std::make_pair(Iter(n, e, g), Iter(nullptr, nullptr, g));
    }
    if (::agisundirected(g)) {
        graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstedge(g, n);
        return std::make_pair(Iter(n, e, g), Iter(nullptr, nullptr, g));
    }
    return std::make_pair(Iter(nullptr, nullptr, g), Iter(nullptr, nullptr, g));
}

graph_traits<cgraph_graph_ptr>::edges_size_type num_edges(const cgraph_graph_ptr & g)
{
    assert(g);
    return ::agnedges(g);
}

// source(e, g) defines in IncidenceGraph
// target(e, g) defined in IncidenceGraph

/* AdjancencyMatrix */

// edge(u, v, g)

/* MutableGraph */

std::pair<graph_traits<cgraph_graph_ptr>::edge_descriptor, bool> add_edge(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, graph_traits<cgraph_graph_ptr>::vertex_descriptor v, cgraph_graph_ptr g)
{
    assert(u);
    assert(v);
    assert(g);
    graph_traits<cgraph_graph_ptr>::edge_descriptor edge = ::agedge(g, u, v, nullptr, 1);
    return std::make_pair(edge, true); // @todo false if strict and edge exists already
}

void remove_edge(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, graph_traits<cgraph_graph_ptr>::vertex_descriptor v, cgraph_graph_ptr g)
{
    assert(u);
    assert(v);
    assert(g);
    graph_traits<cgraph_graph_ptr>::edge_descriptor edge = ::agedge(g, u, v, nullptr, 0);
    if (edge) {
        ::agdeledge(g, edge);
    }
}

void remove_edge(graph_traits<cgraph_graph_ptr>::edge_descriptor e, cgraph_graph_ptr g)
{
    assert(e);
    assert(g);
    ::agdeledge(g, e);
}

void remove_edge(graph_traits<cgraph_graph_ptr>::out_edge_iterator iter, cgraph_graph_ptr g)
{
    assert(*iter);
    assert(g);
    ::agdeledge(g, *iter);
}

template<typename Predicate>
void remove_edge_if(Predicate p, cgraph_graph_ptr g)
{
    // assert(p);
    assert(g);
    for (graph_traits<cgraph_graph_ptr>::vertex_descriptor v = ::agfstnode(g); v; v = ::agnxtnode(g, v)) {
        for (graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstout(g, v); e; e = ::agnxtout(g, e)) {
            if (p(e)) {
                ::agdeledge(g, e);
            }
        }
    }
}

template<typename Predicate>
void remove_out_edge_if(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, Predicate p, cgraph_graph_ptr g)
{
    assert(u);
    // assert(p);
    assert(g);
    for (graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstout(g, u); e; e = ::agnxtout(g, e)) {
        if (p(e)) {
            ::agdeledge(g, e);
        }
    }
}

template<typename Predicate>
void remove_in_edge_if(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, Predicate p, cgraph_graph_ptr g)
{
    assert(u);
    // assert(p);
    assert(g);
    for (graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstin(g, u); e; e = ::agnxtin(g, e)) {
        if (p(e)) {
            ::agdeledge(g, e);
        }
    }
}

graph_traits<cgraph_graph_ptr>::vertex_descriptor add_vertex(cgraph_graph_ptr g)
{
    assert(g);
    return ::agnode(g, nullptr, 1);
}

void clear_vertex(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, cgraph_graph_ptr g)
{
    assert(v);
    assert(g);
    for (graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstout(g, v); e; e = ::agnxtout(g, e)) {
        ::agdeledge(g, e);
    }
    for (graph_traits<cgraph_graph_ptr>::edge_descriptor e = ::agfstin(g, v); e; e = ::agnxtin(g, e)) {
        ::agdeledge(g, e);
    }
}

void remove_vertex(graph_traits<cgraph_graph_ptr>::vertex_descriptor v, cgraph_graph_ptr g)
{
    assert(v);
    assert(g);
    ::agdelnode(g, v);
}

/* PropertyGraph */

class cgraph_graph_id_map :
    public put_get_helper<::IDTYPE, cgraph_graph_id_map>
{
public:
    typedef readable_property_map_tag category;
    typedef ::IDTYPE value_type;
    typedef ::IDTYPE reference;
    typedef void * key_type;

    cgraph_graph_id_map()
    {
    }

    ::IDTYPE operator[](void * x) const
    {
        assert(x);
        return AGID(x);
    }
};

inline cgraph_graph_id_map get(edge_index_t /*p*/, const cgraph_graph_ptr & g)
{
    // assert(p);
    assert(g);
    return cgraph_graph_id_map();
}

inline ::IDTYPE get(edge_index_t p, const cgraph_graph_ptr & g, const graph_traits<cgraph_graph_ptr>::edge_descriptor & x)
{
    // assert(p);
    assert(g);
    assert(x);
    return get(get(p, g), x);
}

inline cgraph_graph_id_map get(vertex_index_t /*p*/, const cgraph_graph_ptr & g)
{
    // assert(p);
    assert(g);
    return cgraph_graph_id_map();
}

inline ::IDTYPE get(vertex_index_t p, const cgraph_graph_ptr & g, const graph_traits<cgraph_graph_ptr>::vertex_descriptor & x)
{
    // assert(p);
    assert(g);
    assert(x);
    return get(get(p, g), x);
}

template<>
struct property_map<cgraph_graph_ptr, edge_index_t>
{
    typedef cgraph_graph_id_map type;
    typedef cgraph_graph_id_map const_type;
};

template<>
struct property_map<cgraph_graph_ptr, vertex_index_t>
{
    typedef cgraph_graph_id_map type;
    typedef cgraph_graph_id_map const_type;
};

class cgraph_graph_name_map :
    public put_get_helper<char *, cgraph_graph_name_map>
{
public:
    typedef readable_property_map_tag category;
    typedef char * value_type;
    typedef char * reference;
    typedef void * key_type;

    cgraph_graph_name_map()
    {
    }

    char * operator[](void * x) const
    {
        assert(x);
        return ::agnameof(x);
    }
};

inline cgraph_graph_name_map get(graph_name_t /*p*/, const cgraph_graph_ptr & g)
{
    // assert(p);
    assert(g);
    return cgraph_graph_name_map();
}

inline char * get(graph_name_t p, const cgraph_graph_ptr & g, const cgraph_graph_ptr & x)
{
    // assert(p);
    assert(g);
    assert(x);
    return get(get(p, g), x);
}

inline cgraph_graph_name_map get(edge_name_t /*p*/, const cgraph_graph_ptr & g)
{
    // assert(p);
    assert(g);
    return cgraph_graph_name_map();
}

inline char * get(edge_name_t p, const cgraph_graph_ptr & g, const graph_traits<cgraph_graph_ptr>::edge_descriptor & x)
{
    // assert(p);
    assert(g);
    assert(x);
    return get(get(p, g), x);
}

inline cgraph_graph_name_map get(vertex_name_t /*p*/, const cgraph_graph_ptr & g)
{
    // assert(p);
    assert(g);
    return cgraph_graph_name_map();
}

inline char * get(vertex_name_t p, const cgraph_graph_ptr & g, const graph_traits<cgraph_graph_ptr>::vertex_descriptor & x)
{
    // assert(p);
    assert(g);
    assert(x);
    return get(get(p, g), x);
}

template<>
struct property_map<cgraph_graph_ptr, graph_name_t>
{
    typedef cgraph_graph_name_map type;
    typedef cgraph_graph_name_map const_type;
};

template<>
struct property_map<cgraph_graph_ptr, edge_name_t>
{
    typedef cgraph_graph_name_map type;
    typedef cgraph_graph_name_map const_type;
};

template<>
struct property_map<cgraph_graph_ptr, vertex_name_t>
{
    typedef cgraph_graph_name_map type;
    typedef cgraph_graph_name_map const_type;
};

// @todo how are all the other graphviz properties be handled?

/* MutablePropertyGraph */

template<>
struct edge_property_type<cgraph_graph_ptr>
{
    typedef graph_traits<cgraph_graph_ptr>::edge_property_type type;
};

std::pair<graph_traits<cgraph_graph_ptr>::edge_descriptor, bool> add_edge(graph_traits<cgraph_graph_ptr>::vertex_descriptor u, graph_traits<cgraph_graph_ptr>::vertex_descriptor v, graph_traits<cgraph_graph_ptr>::edge_property_type ep, cgraph_graph_ptr g)
{
    assert(u);
    assert(v);
    assert(ep);
    assert(g);
    graph_traits<cgraph_graph_ptr>::edge_descriptor edge = ::agedge(g, u, v, ep, 1);
    return std::make_pair(edge, true); // @todo false if strict and edge exists already
}

template<>
struct vertex_property_type<cgraph_graph_ptr>
{
    typedef graph_traits<cgraph_graph_ptr>::vertex_property_type type;
};

graph_traits<cgraph_graph_ptr>::vertex_descriptor add_vertex(graph_traits<cgraph_graph_ptr>::vertex_property_type vp, cgraph_graph_ptr g)
{
    assert(vp);
    assert(g);
    return ::agnode(g, vp, 1);
}

} // namespace boost

/* @todo got concept check failures (fpermissive) without this... */
using boost::out_edges;
using boost::source;
using boost::target;
using boost::out_degree;
using boost::in_edges;
using boost::in_degree;
using boost::degree;
using boost::adjacent_vertices;
using boost::vertices;
using boost::num_vertices;
using boost::edges;
using boost::num_edges;
using boost::add_vertex;
using boost::clear_vertex;
using boost::remove_vertex;
using boost::add_edge;
using boost::remove_edge;
using boost::get;
using boost::put;

#endif // BOOST_GRAPH_CGRAPH_GRAPH_HPP
