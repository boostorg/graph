#ifndef BOOST_SHADOW_ITERATOR_HPP
#define BOOST_SHADOW_ITERATOR_HPP

#include <boost/iterator_adaptors.hpp>
#include <boost/operators.hpp>

namespace boost {

  namespace detail {

    template <class A, class B, class D>
    class shadow_proxy
      : boost::operators< shadow_proxy<A,B,D> >
    {
      typedef shadow_proxy self;
    public:
      inline shadow_proxy(A aa, B bb) : a(aa), b(bb) { }
      inline shadow_proxy(const self& x) : a(x.a), b(x.b) { }
      template <class Self>
      inline shadow_proxy(Self x) : a(x.a), b(x.b) { }
      inline self& operator=(const self& x) { a = x.a; b = x.b; return *this; }
      inline self& operator++() { ++a; return *this; }
      inline self& operator--() { --a; return *this; }
      inline self& operator+=(const self& x) { a += x.a; return *this; }
      inline self& operator-=(const self& x) { a -= x.a; return *this; }
      inline self& operator*=(const self& x) { a *= x.a; return *this; }
      inline self& operator/=(const self& x) { a /= x.a; return *this; }
      inline self& operator%=(const self& x) { return *this; } // JGS
      inline self& operator&=(const self& x) { return *this; } // JGS
      inline self& operator|=(const self& x) { return *this; } // JGS
      inline self& operator^=(const self& x) { return *this; } // JGS
      inline friend D operator-(const self& x, const self& y) {
	return x.a - y.a;
      } 
      inline bool operator==(const self& x) const { return a == x.a;  }
      inline bool operator<(const self& x) const { return a < x.a;  }
      //  protected:
      A a;
      B b;
    };

    struct shadow_iterator_policies
    {
      template <typename iter_pair>
      void initialize(const iter_pair&) { }

      template <typename R, typename iter_pair>
      R dereference(type<R>, const iter_pair& p) const { 
	return R(*p.first, *p.second); 
      }
      template <typename iter_pair>
      bool equal(const iter_pair& p1, const iter_pair& p2) const { 
	return p1.first == p2.first;  
      }
      template <typename iter_pair>
      void increment(iter_pair& p) { ++p.first; ++p.second; }

      template <typename iter_pair>
      void decrement(iter_pair& p) { --p.first; --p.second; }

      template <typename iter_pair>
      bool less(const iter_pair& x, const iter_pair& y) const { 
	return x.first < y.first;  
      }
      template <typename D, typename iter_pair>
      D distance(type<D>, const iter_pair& x, const iter_pair& y) const { 
	return y.first - x.first; 
      }
      template <typename D, typename iter_pair>
      void advance(iter_pair& p, D n) { p.first += n; p.second += n; }
    };

  } // namespace detail

  template <typename IterA, typename IterB>
  struct shadow_iterator_generator {
    typedef typename std::iterator_traits<IterA>::value_type Aval;
    typedef typename std::iterator_traits<IterB>::value_type Bval;
    typedef typename std::iterator_traits<IterA>::reference Aref;
    typedef typename std::iterator_traits<IterB>::reference Bref;
    typedef typename std::iterator_traits<IterA>::difference_type D;
    typedef detail::shadow_proxy<Aval,Bval,Aval> V;
    typedef detail::shadow_proxy<Aref,Bref,Aval> R;
    typedef iterator_adaptor< std::pair<IterA, IterB>,
			      detail::shadow_iterator_policies,
			      V, R, V*, std::random_access_iterator_tag,
			      D> type;
  };

  // short cut for creating a shadow iterator
  template <class IterA, class IterB>
  inline typename shadow_iterator_generator<IterA,IterB>::type
  make_shadow_iter(IterA a, IterB b) {
    typedef typename shadow_iterator_generator<IterA,IterB>::type Iter;
    return Iter(std::make_pair(a,b)); 
  }

  template <class Cmp>
  struct shadow_cmp {
    inline shadow_cmp(const Cmp& c) : cmp(c) { }
    template <class ShadowProxy1, class ShadowProxy2>
    inline bool operator()(const ShadowProxy1& x, const ShadowProxy2& y) const
    {
      return cmp(x.a, y.a);
    }
    Cmp cmp;
  };

} // namespace boost

namespace std {
  template <class A1, class B1, class D1,
	    class A2, class B2, class D2>
  void swap(boost::detail::shadow_proxy<A1&,B1&,D1> x,
	    boost::detail::shadow_proxy<A2&,B2&,D2> y)
  {
    std::swap(x.a, y.a);
    std::swap(x.b, y.b);
  }
}

#endif // BOOST_SHADOW_ITERATOR_HPP
