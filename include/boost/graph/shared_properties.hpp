// (C) Copyright Andrew Sutton 2008-2009
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_SHARED_PROPERTIES_HPP
#define BOOST_GRAPH_SHARED_PROPERTIES_HPP

#include <boost/shared_ptr.hpp>

#include <boost/graph/detail/hashed_property_container.hpp>
#include <boost/graph/detail/indexed_property_container.hpp>

namespace boost {
namespace graph_detail {
    // Define the mapping strategy based on the type of descriptor. By default,
    // we prefer to use hashing since the number of data structures that
    // actually index their vertices is tiny.
    struct index_mapping { };
    struct hash_mapping { };

    /** @internal @name Descriptor Mapping */
    //@{
    template <typename Descriptor>
    struct descriptor_mapping
    { typedef hash_mapping strategy; };

    // If the descriptor is an unsigned int, then we can use a vector.
    template <>
    struct descriptor_mapping<std::size_t>
    { typedef index_mapping strategy; };
    //@}

    // Select the type of container based on the underlying store selector.
    // Noe that these have to wrap the underlying containers so that a common
    // interface exists between the two.
    template <typename Descriptor, typename Property>
    struct choose_container
        : typename mpl::if_<
            is_same<
                typename descriptor_mapping<Descriptor>::strategy, hash_mapping
            >,
            hashed_property_container<Descriptor, Property>,
            indexed_property_container<Descriptor, Property>
        >::type type;
    { };

    /** @internal @name Get Range
     * Return a range over the set of vertices or edges. Note that this is
     * going to be truly problematic if is_same<Vertex, Edge>.
     */
    template <typename Graph>
    inline typename Graph::vertex_range
    get_range(Graph const& g, typename Graph::vertex_descriptor)
    { return g.vertices(); }

    template <typename Graph>
    inline typename Graph::edge_range
    get_range(Graph const& g, typename Graph::edge_descriptor)
    { return g.edges(); }
} // namespace graph_detail

// TODO: Work in progress.

/**
 * The label type allows the definition of exterior properties that
 * can maintain either their own internal store (via shared pointers) or be
 * constructed over an exterioir store. This is useful in algorithms that
 * can provide default exterior properties or allow the user to provide their
 * own.
 *
 * The use of this type incurs a slight overhead due to an additional level of
 * indirection.
 */
template <typename Graph, typename Descriptor, typename Label>
struct label {
    // Select the container and map type for the self-wrapping property.
    typedef typename detail::choose_container<
        Descriptor, Label
    >::type Container;

    typedef typename Label value_type;
    typedef typename Label& reference;
    typedef typename Descriptor key_type;

    // By default, the optional property contains no property. This should
    // probably never be used.
    label()
        : data()
    { }

    // Allocate a label and build a property map over it.
    label(Graph const& g, value_type const& x = value_type())
        : container(new Container(detail::get_range(g, Descriptor()), x))
    { }

    // Construct the optional property over the given map, without allocating
    // an exterior label.
    label(Container&  cont)
        : container(&cont)
    { }

    value_type& operator()(key_type const& key)
    { return map(key); }

    void operator()(key_type const& key, value_type const& value) const
    { return map(key, value); }

    optional_label& swap(optional_label& x) {
        using std::swap;
        swap(container, x.container);   // Should overload to constant time op.
        swap(map, x.map);
        return *this;
    }

    shared_ptr<Container> data;
};

/**
 * The optional vertex map allows a user-provided property map or a self-
 * contained exterior property to be passed to a generic function. The user
 * provided property map is not required to be constructed over an exterior
 * property.
 */
template <typename Graph, typename Label>
struct vertex_label
    : label<Graph, typename Graph::vertex_descriptor, Label>
{
    typedef label<Graph, typename Graph::vertex_descriptor, Label> Base;
    typedef typename Base::Container Container;

    vertex_label() : Base() { }
    vertex_label(Graph const& g, Label const& x = Label()) : Base(g, x) { }
    vertex_label(Container& cont) : Base(cost) { }
};

/**
 * The optional edge map allows a user-provided property map or a self-
 * contained exterior property to be passed to a generic function. The user
 * provided property map is not required to be constructed over an exterior
 * property.
 */
template <typename Graph, typename Label>
struct edge_label
    : label<Graph, typename Graph::edge_descriptor, Label>
{
    typedef label<Graph, typename Graph::vertex_descriptor, Label> base_type;
    typedef typename Base::Container Container;

    edge_label() : Base() { }
    edge_label(Graph const& g, Label const& x = Label()) : Base(g, x) { }
    edge_label(Container& cont) : Base(cont) { }
};

namespace detail
{
    // Optionally initialize the container, but not if the map is already
    // initialized.
    template <typename Graph, typename Map>
    void optional_init(Graph const& g, Map& map, typename Map::value_type x)
    {
        if(!map.data) {
            Map tmp(g, x);
            map.swap(tmp);
        }
    }
}

/** @name Initialize Property Map
 * Delayed initialization of optional property maps. The default solution
 * is to do nothing (i.e,. the map is already initialized). Specialized
 * variants simply swap the given map with one that's actually initialized.
 */
//@{
/*
template <typename Graph, typename Map>
void initialize(Graph const&, Map&, typename Map::value_type)
{ throw 0; }
*/

template <typename Graph, typename Label>
void initialize(Graph const& g, optional_vertex_label<Graph, Label>& map, Label const& x)
{ detail::optional_init(g, map, x); }

template <typename Graph, typename Label>
void initialize(Graph const g, optional_edge_label<Graph, Label>& map, Label const& x)
{ detail::optional_init(g, map, x); }
//@}

} // namespace boost

#endif
