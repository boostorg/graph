#ifndef BOOST_ADJACENCY_ITERATOR_HPP
#define BOOST_ADJACENCY_ITERATOR_HPP

#include <boost/iterator_adaptors.hpp>
#include <boost/graph/graph_traits.hpp>

namespace boost {

  namespace detail {

    template <class Graph>
    struct adjacency_iterator_policies : 
      public boost::default_iterator_policies
    {
      inline adjacency_iterator_policies() { }
      inline adjacency_iterator_policies(const Graph* g) : m_g(g) { }

      template <class Iterator>
      inline typename Iterator::reference
      dereference(const Iterator& i) const
        { return target(*i.base(), *m_g); }

      const Graph* m_g;
    };

  } // namespace detail

  template <class Graph,
            class Vertex = typename graph_traits<Graph>::vertex_descriptor,
            class OutEdgeIter=typename graph_traits<Graph>::out_edge_iterator>
  class adjacency_iterator_generator {
    typedef typename boost::detail::iterator_traits<OutEdgeIter>
      ::difference_type difference_type;
  public:
    typedef boost::iterator_adaptor<OutEdgeIter, 
      detail::adjacency_iterator_policies<Graph>,
      Vertex, Vertex, Vertex*, boost::multi_pass_input_iterator_tag,
      difference_type
    > type;
  };

  namespace detail {

    template <class Graph>
    struct inv_adjacency_iterator_policies : 
      public boost::default_iterator_policies
    {
      inline inv_adjacency_iterator_policies() { }
      inline inv_adjacency_iterator_policies(Graph* g) : m_g(g) { }

      template <class Iterator>
      inline typename Iterator::reference
      dereference(const Iterator& i) const
        { return source(*i.base(), *m_g); }

      Graph* m_g;
    };

  } // namespace detail

  template <class Graph, class Vertex, class InEdgeIter>
  class inv_adjacency_iterator_generator {
    typedef typename boost::detail::iterator_traits<InEdgeIter>
      ::difference_type difference_type;
  public:
    typedef boost::iterator_adaptor<InEdgeIter, 
      detail::inv_adjacency_iterator_policies<Graph>,
      Vertex, Vertex, Vertex*, boost::multi_pass_input_iterator_tag,
      difference_type
    > type;
  };

} // namespace boost

#endif // BOOST_DETAIL_ADJACENCY_ITERATOR_HPP
