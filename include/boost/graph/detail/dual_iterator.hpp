#ifndef BOOST_DUAL_ITERATOR_HPP
#define BOOST_DUAL_ITERATOR_HPP

#include <boost/iterator_adaptors.hpp>
#include <boost/operators.hpp>

namespace boost {

  namespace detail {

    template <class A, class B, class D>
    class dual_proxy
      : boost::operators< dual_proxy<A,B,D> >
    {
      typedef dual_proxy self;
    public:
      inline dual_proxy(A aa, B bb) : a(aa), b(bb) { }
      inline dual_proxy(const self& x) : a(x.a), b(x.b) { }
      template <class Self>
      inline dual_proxy(Self x) : a(x.a), b(x.b) { }
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

    struct dual_iterator_policies
    {
      template <typename R, typename iter_pair>
      inline R dereference(type<R>&, const iter_pair& p) const { 
	return R(*p.first, *p.second); 
      }
      template <typename iter_pair>
      bool equal(const iter_pair& p1, const iter_pair& p2) const { 
	return p1.first == p2.first;  
      }
      template <typename iter_pair>
      void increment(const iter_pair& p) { ++p.first; ++p.second; }

      template <typename iter_pair>
      void decrement(const iter_pair& p) { --p.first; --p.second; }

      template <typename iter_pair>
      bool less(const iter_pair& p1, const iter_pair& p2) const { 
	return p1.first < p2.first;  
      }
      template <typename D, typename iter_pair>
      D distance(type<D>, const iter_pair& p1, const iter_pair& p2) const { 
	return p1.first - p2.first; 
      }
      template <typename D, typename iter_pair>
      void advance(iter_pair& p, D n) { p.first += n; p.second += n; }
    };

  } // namespace detail

  template <typename IterA, typename IterB>
  struct dual_iterator_generator {
    typedef typename std::iterator_traits<IterA>::value_type Aval;
    typedef typename std::iterator_traits<IterB>::value_type Bval;
    typedef typename std::iterator_traits<IterA>::reference Aref;
    typedef typename std::iterator_traits<IterB>::reference Bref;
    typedef typename std::iterator_traits<IterA>::difference_type D;
    typedef detail::dual_proxy<Aval,Bval,Aval> V;
    typedef detail::dual_proxy<Aref,Bref,Aval> R;
    typedef iterator_adaptor< std::pair<IterA, IterB>,
			      detail::dual_iterator_policies,
			      V, R, V*, std::random_access_iterator_tag,
			      D> type;
  };

  // short cut for creating a dual iterator
  template <class IterA, class IterB>
  inline typename dual_iterator_generator<IterA,IterB>::type
  make_dual_iter(IterA a, IterB b) {
    typedef typename dual_iterator_generator<IterA,IterB>::type Iter;
    return Iter(std::make_pair(a,b)); 
  }

  template <class Cmp>
  struct dual_cmp {
    inline dual_cmp(const Cmp& c) : cmp(c) { }
    template <class DualProxy1, class DualProxy2>
    inline bool operator()(const DualProxy1& x, const DualProxy2& y) const {
      return cmp(x.a, y.a);
    }
    Cmp cmp;
  };

} // namespace boost


#endif // BOOST_DUAL_ITERATOR_HPP
