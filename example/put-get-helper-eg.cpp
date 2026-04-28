//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee,
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <vector>
#include <string>
#include <boost/property_map/property_map.hpp>
#include <boost/concept/assert.hpp>

#ifdef BOOST_NO_STD_ITERATOR_TRAITS
#error This examples requires a compiler that provides a working std::iterator_traits
#endif

namespace foo
{
using namespace boost;
template < class RandomAccessIterator, class IndexMap >
class iterator_property_map
: public boost::put_get_helper<
      typename std::iterator_traits< RandomAccessIterator >::reference,
      iterator_property_map< RandomAccessIterator, IndexMap > >
{
public:
    using key_type = std::ptrdiff_t;
    using value_type =
        typename std::iterator_traits< RandomAccessIterator >::value_type;
    using reference =
        typename std::iterator_traits< RandomAccessIterator >::reference;
    using category = boost::lvalue_property_map_tag;

    iterator_property_map(RandomAccessIterator cc = RandomAccessIterator(),
        const IndexMap& _id = IndexMap())
    : iter(cc), index(_id)
    {
    }
    reference operator[](std::ptrdiff_t v) const
    {
        return *(iter + get(index, v));
    }

protected:
    RandomAccessIterator iter;
    IndexMap index;
};

}

int main()
{
    using vec_t = std::vector< std::string >;
    using pmap_t = foo::iterator_property_map< vec_t::iterator,
        boost::identity_property_map >;
    using namespace boost;
    BOOST_CONCEPT_ASSERT((Mutable_LvaluePropertyMapConcept< pmap_t, int >));
    return 0;
}
