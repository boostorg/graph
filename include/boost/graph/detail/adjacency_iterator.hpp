#ifndef BOOST_DETAIL_ADJACENCY_ITERATOR_HPP
#define BOOST_DETAIL_ADJACENCY_ITERATOR_HPP

#include <boost/pending/iterator_adaptors.hpp>

namespace boost {

  namespace detail {

#ifndef BOOST_NO_ITERATOR_ADAPTORS
    template <class Vertex, class Traits>
    struct adjacency_iterator_traits {
      typedef Vertex value_type;
      typedef value_type reference;
      typedef value_type* pointer;
      typedef boost::multi_pass_input_iterator_tag iterator_category;
      typedef typename Traits::difference_type difference_type;
    };

    template <class Graph>
    struct adjacency_iterator_policies : 
      public boost::default_iterator_policies
    {
      inline adjacency_iterator_policies() { }
      inline adjacency_iterator_policies(Graph* g) : m_g(g) { }

      template <class Reference, class Iterator>
      inline Reference
      dereference(boost::type<Reference>, const Iterator& i) const
        { return target(*i, *m_g); }

      Graph* m_g;
    };

    template <class Graph, class Vertex, class OutEdgeIter,
#if !defined BOOST_NO_STD_ITERATOR_TRAITS
              class Traits = std::iterator_traits<OutEdgeIter>
#else
              class Traits
#endif
             >
    struct adjacency_iterator {
      typedef boost::iterator_adaptor<OutEdgeIter, 
        adjacency_iterator_policies<Graph>,
        adjacency_iterator_traits<Vertex, Traits> 
      > type;
    };
#else
    template <class Vertex, class OutEdgeIter, class Graph>
    struct bidir_adj_iter
      : public boost::iterator<boost::multi_pass_input_iterator_tag, Vertex,
                               std::ptrdiff_t, Vertex*, Vertex>
    {
    private:
      typedef bidir_adj_iter self;
    public:
      typedef std::ptrdiff_t difference_type;
      typedef boost::multi_pass_input_iterator_tag iterator_category;
      typedef Vertex* pointer;
      typedef Vertex reference;
      typedef Vertex value_type;
      inline bidir_adj_iter() { }
          inline bidir_adj_iter(OutEdgeIter ii, Graph* _g)
        : i(ii), g(_g) {}

      inline self& operator++() { ++i; return *this; }
      inline self operator++(int) { self tmp = *this; ++(*this); return tmp; }
      inline reference operator*() const { return target(*i, *g); }
      /* Attention: */
      /* Even if two iterators are not equal, they could be the same vertex! */
      /* i.e.  i != j does not mean *i != *j */

      inline bool operator!=(const self& x) const { return i != x.i; }
      inline bool operator==(const self& x) const { return i == x.i; }

      inline self* operator->() { return this; }

      OutEdgeIter& iter() { return i; }
      const OutEdgeIter& iter() const { return i; }

      /*    protected: */
      OutEdgeIter i;
      Graph* g;
    protected:
    };
#endif

  } // namespace detail

} // namespace boost

#endif // BOOST_DETAIL_ADJACENCY_ITERATOR_HPP
