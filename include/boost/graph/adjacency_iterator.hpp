#ifndef BOOST_ADJACENCY_ITERATOR_HPP
#define BOOST_ADJACENCY_ITERATOR_HPP

#include <boost/iterator_adaptors.hpp>

namespace boost {

  namespace detail {

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

  } // namespace detail

  template <class Graph, class Vertex, class OutEdgeIter>
  class adjacency_iterator_generator {
    typedef typename boost::detail::iterator_traits<OutEdgeIter>
      ::difference_type difference_type;
    typedef boost::iterator<boost::multi_pass_input_iterator_tag,
      Vertex, difference_type, Vertex*, Vertex> Traits;
  public:
    typedef boost::iterator_adaptor<OutEdgeIter, 
      detail::adjacency_iterator_policies<Graph>, Traits> type;
  };

} // namespace boost

#endif // BOOST_DETAIL_ADJACENCY_ITERATOR_HPP
