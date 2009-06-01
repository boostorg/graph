// (C) Copyright Andrew Sutton 2008-2009
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_GRAPH_DETAIL_HASHED_PROPERTIES_HPP
#define BOOST_GRAPH_DETAIL_HASHED_PROPERTIES_HPP

#include <tr1/unordered_map>
#include <boost/iterator/iterator_facade.hpp>
#include <boost/functional/hash.hpp>

namespace boost { namespace graph_detail {

/**
 * Wrap an iterator with a default value so that it generates default values
 * over a range of keys.
 */
template <typename Iter, typename Prop>
struct key_value_iterator
{
    typedef typename std::forward_iterator_tag iterator_category;
    typedef typename std::size_t difference_type;

    typedef std::pair<typename Iter::value_type, Prop> value_type;
    typedef value_type reference;
    typedef value_type pointer;

    key_value_iterator(Iter i, Prop const& p)
        : iter(i), value(p)
    { }

    key_value_iterator& operator++()
    { ++iter; return *this; }

    reference operator*()
    { return make_pair(*iter, value); }

    bool operator==(key_value_iterator const& x) const
    { return iter == x.iter; }

    bool operator!=(key_value_iterator const& x) const
    { return iter != x.iter; }

    Iter    iter;
    Prop    value;
};

template <typename Iter, typename Prop>
inline key_value_iterator<Iter, Prop>
make_key_value_iterator(Iter i, Prop const& x)
{ return key_value_iterator<Iter, Prop>(i, x); }

/**
 * A simple wrapper around an unordered map, this is used to map descriptors
 * to arbitrary property values. Note that the property type must be default
 * constructible.
 *
 * This may seem a little odd because we're passing an iterator and not the key
 * type. However, the key type is always the iterator's value type.
 */
template <typename Descriptor, typename Property>
class hashed_property_container
{
public:
    typedef Property value_type;
    typedef Descriptor key_type;
    typedef std::tr1::unordered_map<key_type, value_type, boost::hash<key_type>> container_type;

    /**
     * Construct the hashtable over n buckets. This may not actually allocate
     * n buckets, so we can't necessarily guarantee that memory will actually
     * be allocated for each element, much less what those default values would
     * actually be.
     */
    hashed_property_container(std::size_t n)
        : data(n)
    { }

    /**
     * Construct the hashtable over the keys in the iterator range [f, l) with
     * the default value x.
     */
    template <typename Iter>
    hashed_property_container(Iter f, Iter l, value_type const& x)
        : data(detail::make_key_value_iterator(f, x),
               detail::make_key_value_iterator(l, value_type()))
    { }

    template <typename Range>
    hashed_property_container(Range rng, value_type const& x)
        : data(detail::make_key_value_iterator(rng.first, x),
               detail::make_key_value_iterator(rng.second, value_type()))
    { }

    inline value_type& operator[](key_type const& k)
    { return data[k]; }

    inline value_type const& operator[](key_type const& k) const
    { return data[k]; }

    container_type data;
};

} } // namespace boost::graph_detail


#endif
