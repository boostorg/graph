// (C) Copyright Andrew Sutton 2008-2009
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_DETAIL_INDEXED_PROPERTIES_HPP
#define BOOST_GRAPH_DETAIL_INDEXED_PROPERTIES_HPP

#include <vector>
#include <boost/iterator/iterator_facade.hpp>

namespace boost { namespace graph_detail {

/**
 * Wrap an iterator with a default value so that it generates default values
 * over a range of keys. This maintains the defalt value as an object to
 * prevent multiple constructions if the object is "heavy".
 */
template <typename Iter, typename Prop>
struct default_value_iterator
    : iterator_facade<
        default_value_iterator<Iter, Prop>, Prop, std::forward_iterator_tag,
        Prop const&
    >
{
    typedef typename std::forward_iterator_tag iterator_category;
    typedef std::size_t difference_type;

    typedef Prop value_type;
    typedef value_type const& reference;
    typedef value_type const* pointer;

    default_value_iterator(Iter i, Prop const& p)
        : iter(i), value(p)
    { }

    void advance()
    { ++iter; }

    bool equal(default_value_iterator const& x) const
    { return iter == x.iter; }

    Prop const& dereference() const
    { return value; }

    Iter iter;
    Prop value;
};

template <typename Iter, typename Prop>
inline default_value_iterator<Iter, Prop>
make_default_value_iterator(Iter i, Prop const& p)
{ return default_value_iterator<Iter, Prop>(i, p); }

/**
 * A simple wrapper around a vector. Because the "key" to this vector is
 * actually given as a descriptor, we have to get the underlying index that
 * allows us to map this value to a property.
 *
 * @todo If the underlying container is shared, then this can act as both
 * the property map and the container. Or we could have the property map
 * be a shared_ptr to this (or other) containers.
 */
template <typename Descriptor, typename Property>
struct indexed_property_container
{
    typedef Property value_type;
    typedef Descriptor key_type;
    typedef std::vector<Property> container_type;

    inline indexed_property_container(std::size_t n)
        : data(n)
    { }

    /**
     * Construct the hashtable over the keys in the iterator range [f, l) with
     * the default value x.
     */
    template <typename Iter>
    inline indexed_property_container(Iter f, Iter l, value_type const& x)
        : data(detail::make_default_value_iterator(f, x),
               detail::make_default_value_iterator(l, value_type()))
    { }

    template <typename Range>
    inline indexed_property_container(Range rng, value_type const& x)
        : data(detail::make_default_value_iterator(rng.first, x),
               detail::make_default_value_iterator(rng.second, value_type()))
    { }

    inline value_type& operator[](key_type const& k)
    { return data[k.value]; }

    inline value_type const& operator[](key_type const& k) const
    { return data[k.value]; }

    container_type data;
};

} } // namespace boost::graph_detail

#endif
