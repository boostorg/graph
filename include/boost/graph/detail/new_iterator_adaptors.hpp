// (C) Copyright David Abrahams 2000. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
//
// (C) Copyright Jeremy Siek 2000. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.
//
// Revision History:

// 20 Mar 2001   Jeremy Siek
//      Branched off of old iterator_adaptors.hpp with intention of
//      changing this to use the new iterator_traits in
//      development. For now the only difference is that the
//      conformance checks are removed so that shadow_iterator works.

#ifndef BOOST_NEW_ITERATOR_ADAPTOR_DWA053000_HPP_
# define BOOST_NEW_ITERATOR_ADAPTOR_DWA053000_HPP_

# include <boost/iterator_adaptors.hpp>

namespace boost {

// This macro definition is only temporary in this file
# if !defined(BOOST_MSVC)
#  define BOOST_ARG_DEPENDENT_TYPENAME typename
# else
#  define BOOST_ARG_DEPENDENT_TYPENAME
# endif

//============================================================================
//new_iterator_adaptor - Adapts a generic piece of data as an iterator. Adaptation
//      is especially easy if the data being adapted is itself an iterator
//
//   Base - the base (usually iterator) type being wrapped.
//
//   Policies - a set of policies determining how the resulting iterator
//      works.
//
//   Value - if supplied, the value_type of the resulting iterator, unless
//      const. If const, a conforming compiler strips constness for the
//      value_type. If not supplied, iterator_traits<Base>::value_type is used
//
//   Reference - the reference type of the resulting iterator, and in
//      particular, the result type of operator*(). If not supplied but
//      Value is supplied, Value& is used. Otherwise
//      iterator_traits<Base>::reference is used.
//
//   Pointer - the pointer type of the resulting iterator, and in
//      particular, the result type of operator->(). If not
//      supplied but Value is supplied, Value* is used. Otherwise
//      iterator_traits<Base>::pointer is used.
//
//   Category - the iterator_category of the resulting iterator. If not
//      supplied, iterator_traits<Base>::iterator_category is used.
//
//   Distance - the difference_type of the resulting iterator. If not
//      supplied, iterator_traits<Base>::difference_type is used.
template <class Base, class Policies, 
    class Value = detail::default_argument,
    class Reference = BOOST_ARG_DEPENDENT_TYPENAME detail::choose_default_argument<Value>::type,
    class Pointer = BOOST_ARG_DEPENDENT_TYPENAME detail::choose_default_argument<Reference>::type,
    class Category = BOOST_ARG_DEPENDENT_TYPENAME detail::choose_default_argument<Pointer>::type,
    class Distance = BOOST_ARG_DEPENDENT_TYPENAME detail::choose_default_argument<Category>::type
         >
struct new_iterator_adaptor :
#ifdef BOOST_RELOPS_AMBIGUITY_BUG
    iterator_comparisons<
          new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance>,
    typename detail::iterator_adaptor_traits_gen<Base,Value,Reference,Pointer,Category, Distance>::type
 >
#else
    detail::iterator_adaptor_traits_gen<Base,Value,Reference,Pointer,Category,Distance>::type
#endif
{
    typedef new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance> self;
 public:
    typedef typename detail::iterator_adaptor_traits_gen<Base,Value,Reference,Pointer,Category,Distance>::type Traits;

    typedef typename Traits::difference_type difference_type;
    typedef typename Traits::value_type value_type;
    typedef typename Traits::pointer pointer;
    typedef typename Traits::reference reference;
    typedef typename Traits::iterator_category iterator_category;

    typedef Base base_type;
    typedef Policies policies_type;

 private:
  // checks for category conformance...

 public:
    new_iterator_adaptor() { }

    explicit
    new_iterator_adaptor(const Base& it, const Policies& p = Policies())
        : m_iter_p(it, p) {
      policies().initialize(iter());
    }

    template <class Iter2, class Value2, class Pointer2, class Reference2>
    new_iterator_adaptor (
        const new_iterator_adaptor<Iter2,Policies,Value2,Reference2,Pointer2,Category,Distance>& src)
            : m_iter_p(src.iter(), src.policies())
    {
        policies().initialize(iter());
    }

#ifdef BOOST_MSVC
    // This is required to prevent a bug in how VC++ generates
    // the assignment operator for compressed_pair.
    new_iterator_adaptor& operator= (const new_iterator_adaptor& x) {
        m_iter_p = x.m_iter_p;
        return *this;
    }
#endif
    reference operator*() const {
        return policies().dereference(type<reference>(), iter());
    }

#ifdef _MSC_VER
# pragma warning(push)
# pragma warning( disable : 4284 )
#endif

    typename boost::detail::operator_arrow_result_generator<iterator_category,value_type,pointer>::type
    operator->() const
        { return detail::operator_arrow(*this, iterator_category()); }

#ifdef _MSC_VER
# pragma warning(pop)
#endif

    value_type operator[](difference_type n) const
        { return *(*this + n); }
    
    self& operator++() {
#ifdef __MWERKS__
        // Odd bug, MWERKS couldn't  deduce the type for the member template
        // Workaround by explicitly specifying the type.
        policies().increment<Base>(iter());
#else
        policies().increment(iter());
#endif
        return *this;
    }

    self operator++(int) { self tmp(*this); ++*this; return tmp; }
    
    self& operator--() {
        policies().decrement(iter());
        return *this;
    }
    
    self operator--(int) { self tmp(*this); --*this; return tmp; }

    self& operator+=(difference_type n) {
        policies().advance(iter(), n);
        return *this;
    }
  
    self& operator-=(difference_type n) {
        policies().advance(iter(), -n);
        return *this;
    }

    base_type base() const { return m_iter_p.first(); }

    // Moved from global scope to avoid ambiguity with the operator-() which
    // subtracts iterators from one another.
    self operator-(difference_type x) const
        { self result(*this); return result -= x; }
private:
    compressed_pair<Base,Policies> m_iter_p;

public: // implementation details (too many compilers have trouble when these are private).
    Policies& policies() { return m_iter_p.second(); }
    const Policies& policies() const { return m_iter_p.second(); }
    
    Base& iter() { return m_iter_p.first(); }
    const Base& iter() const { return m_iter_p.first(); }
};

template <class Base, class Policies, class Value, class Reference, class Pointer,
    class Category, class Distance1, class Distance2>
new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance1>
operator+(
    new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance1> p,
    Distance2 x)
{
    return p += x;
}

template <class Base, class Policies, class Value, class Reference, class Pointer,
    class Category, class Distance1, class Distance2>
new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance1>
operator+(
    Distance2 x,
    new_iterator_adaptor<Base,Policies,Value,Reference,Pointer,Category,Distance1> p)
{
    return p += x;
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2, class Category,
    class Distance>
typename new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>::difference_type
operator-(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
  typedef typename new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,
    Pointer1,Category,Distance>::difference_type difference_type;
  return x.policies().distance(type<difference_type>(), y.iter(), x.iter());
}

#ifndef BOOST_RELOPS_AMBIGUITY_BUG
template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator==(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
    return x.policies().equal(x.iter(), y.iter());
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator<(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
    return x.policies().less(x.iter(), y.iter());
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator>(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{ 
    return x.policies().less(y.iter(), x.iter());
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator>=(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
    return !x.policies().less(x.iter(), y.iter());
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator<=(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x,
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
    return !x.policies().less(y.iter(), x.iter());
}

template <class Iterator1, class Iterator2, class Policies, class Value1, class Value2,
    class Reference1, class Reference2, class Pointer1, class Pointer2,
    class Category, class Distance>
inline bool 
operator!=(
    const new_iterator_adaptor<Iterator1,Policies,Value1,Reference1,Pointer1,Category,Distance>& x, 
    const new_iterator_adaptor<Iterator2,Policies,Value2,Reference2,Pointer2,Category,Distance>& y)
{
    return !x.policies().equal(x.iter(), y.iter());
}
#endif


} // namespace boost
# undef BOOST_ARG_DEPENDENT_TYPENAME


#endif



