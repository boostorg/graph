// Copyright Jeremy Siek 2001-2004
//
// Use, modification and distribution are subject to the Boost Software 
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy 
// at http://www.boost.org/LICENSE_1_0.txt)

#ifndef BOOST_COMPACT_GRAPH_HPP
#define BOOST_COMPACT_GRAPH_HPP

#include <boost/config.hpp>
#include <vector>
#include <boost/iterator_adaptors.hpp>
#include <boost/counting_iterator.hpp>
#include <boost/detail/algorithm.hpp>
#include <boost/graph/adjacency_iterator.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/adjacency_list.hpp> // for directedS, etc.
                // need to split that out from adjacency_list.hpp
#include <boost/graph/detail/permutation.hpp>
#include <boost/detail/select_type.hpp>
#include <boost/type_traits/same_traits.hpp>

namespace boost {

  namespace detail {

    struct reverse_cmp {
      template <typename X1, typename X2, typename Y1, typename Y2>
      bool operator()(const std::pair<X1,X2>& x, 
                      const std::pair<Y1,Y2>& y) const {
        return x.second < y.second 
          || (!(y.second < x.second) && x.first < y.first); 
      }
    };

    struct compact_graph_base
    {
      typedef std::size_t vertices_size_type;
      typedef std::size_t edges_size_type;
      typedef std::vector< std::pair<std::size_t,std::size_t> > EdgeList;
    public:
      template <typename EdgeIterator>
      compact_graph_base(EdgeIterator first, EdgeIterator last,
                         vertices_size_type n, edges_size_type m)
        : point(n + 1)
      {
        edge_list.reserve(m);
        bool sorted = true;
        std::size_t prev = 0;
        for (; first != last; ++first) {
          if ((*first).first < prev)
            sorted = false;
          edge_list.push_back(std::make_pair(first->first, first->second));
          prev = (*first).first;
        }
        if (sorted == false)
          boost::sort(edge_list);

        std::size_t u = 0, e_num = 0;
        point[u] = 0;
        for (typename EdgeList::iterator i = edge_list.begin();
             i != edge_list.end(); ++i, ++e_num)
          if ((*i).first != u) {
            u = (*i).first;
            point[u] = e_num;
          }
        point[u+1] = e_num;
      }
      //protected:
      std::vector<std::size_t> point;
      EdgeList edge_list;
    };

    struct compact_bidir_graph_base : public compact_graph_base
    {
      typedef std::vector< std::pair<std::size_t,std::size_t> > EdgeList;
    public:
      typedef std::size_t vertices_size_type;
      typedef std::size_t edges_size_type;
      template <typename EdgeIterator>
      compact_bidir_graph_base(EdgeIterator first, EdgeIterator last,
                               vertices_size_type n, edges_size_type m)
        : compact_graph_base(first, last, n, m), 
          trace(edge_list.size()),
          rev_point(n + 1)
      {
        // Construct trace array, which stores the permutation from
        // the reverse edges to the real edges
        boost::sortp(edge_list.begin(), edge_list.end(), trace.begin(), 
                     reverse_cmp());
        boost::invert_permutation(trace.begin(), trace.end());
        
        std::size_t v = 0, e_num = 0;
        rev_point[v] = 0;
        for (typename EdgeList::iterator i = edge_list.begin();
             i != edge_list.end(); ++i, ++e_num)
          if ((*i).second != v) {
            v = (*i).second;
            rev_point[v] = e_num;
          }
        rev_point[v+1] = e_num;

        // Permute the edge_list back to normal
        boost::permute(edge_list.begin(), edge_list.end(), trace.begin());
      }
      // invert_permutation requires signed int
      std::vector<std::ptrdiff_t> trace;
      std::vector<std::size_t> rev_point;
    };

    template <typename DirectedS>
    struct compact_graph_choose_dir { 
      typedef typename boost::detail::if_true<
        boost::is_same<DirectedS, boost::bidirectionalS>::value>
        ::template then<
        compact_bidir_graph_base, compact_graph_base>::type type;
    };

    template <typename DirectedS, typename EdgeProperties>
    class compact_graph_edge_prop_base
      : public compact_graph_choose_dir<DirectedS>::type
    {
      typedef std::size_t vertices_size_type;
      typedef std::size_t edges_size_type;
    public:
      template <typename EdgeIterator>
      compact_graph_edge_prop_base(EdgeIterator first, EdgeIterator last,
                              vertices_size_type n, edges_size_type m)
        : compact_graph_base(first, last, n, m), 
          edge_property(edge_list.size())
      { }

      template <typename EdgeIterator, typename EdgePropertyIterator>
      compact_graph_edge_prop_base(EdgeIterator first, EdgeIterator last,
                              EdgePropertyIterator ep_iter,
                              vertices_size_type n, edges_size_type m)
        : compact_graph_base(first, last, n, m), 
          edge_property(edge_list.size())
      {
        for (std::size_t i = 0; i < edge_property.size(); ++i)
          edge_property[i] = *ep_iter++;
      }
      //protected:
      std::vector<EdgeProperties> edge_property;
    };    

    template <typename DirectedS>
    class compact_graph_no_edge_prop_base
      : public compact_graph_choose_dir<DirectedS>::type
    {
      typedef typename compact_graph_choose_dir<DirectedS>::type Base;
      typedef std::size_t vertices_size_type;
      typedef std::size_t edges_size_type;
    public:
      template <typename EdgeIterator>
      compact_graph_no_edge_prop_base(EdgeIterator first, EdgeIterator last,
                                      vertices_size_type n, edges_size_type m)
        : Base(first, last, n, m)
      { }
    };

    template <typename DirectedS, typename EdgeProperties>
    struct compact_graph_choose_edge_prop
    {
      typedef typename boost::detail::if_true<
        boost::is_same<EdgeProperties, boost::no_property>::value>
        ::template then<
        compact_graph_no_edge_prop_base<DirectedS>,
        compact_graph_edge_prop_base<DirectedS, EdgeProperties> >::type type;
    };
    
    template <typename VertexProperties>
    class compact_graph_vertex_prop_base {
      typedef std::size_t vertices_size_type;
    public:
      compact_graph_vertex_prop_base(vertices_size_type n)
        : vertex_property(n) { }
      //protected:
      std::vector<VertexProperties> vertex_property;
    };
    template <>
    class compact_graph_vertex_prop_base<boost::no_property> {
      typedef std::size_t vertices_size_type;
    public:
      compact_graph_vertex_prop_base(vertices_size_type) { }
    };

  } // namespace detail


  template <typename DirectedS = boost::directedS,
            typename VertexProperties = boost::no_property,
            typename EdgeProperties = boost::no_property>
  class compact_graph
    : public detail::compact_graph_choose_edge_prop<DirectedS,
        EdgeProperties>::type,
      public detail::compact_graph_vertex_prop_base<VertexProperties>
  {
    typedef typename detail::compact_graph_choose_edge_prop<DirectedS,
        EdgeProperties>::type EdgeBase;
    typedef detail::compact_graph_vertex_prop_base<VertexProperties>
      VertexBase;
      typedef std::vector< std::pair<std::size_t,std::size_t> > EdgeList;
  public:
    typedef std::size_t vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef std::size_t degree_size_type;
    typedef std::size_t vertices_size_type;
    typedef std::size_t edges_size_type;
    typedef boost::directed_tag directed_category;
    typedef boost::allow_parallel_edge_tag edge_parallel_category;
    struct traversal_category : public boost::incidence_graph_tag,
                                public boost::adjacency_graph_tag, 
                                public boost::vertex_list_graph_tag,
                                public boost::edge_list_graph_tag {};
    typedef VertexProperties vertex_property_type;
    typedef EdgeProperties edge_property_type;
    struct graph_tag { };
    //private:
    struct out_edge_iter_policies : public boost::default_iterator_policies {
      out_edge_iter_policies() { }
      out_edge_iter_policies(const compact_graph* g) : g(g) { }
      template <class Iter>
      const edge_descriptor& dereference(const Iter& x) const {
        return g->edge_list[x.base()];
      }
      const compact_graph* g;
    };
    struct in_edge_iter_policies : public boost::default_iterator_policies {
      in_edge_iter_policies() { }
      in_edge_iter_policies(const compact_graph* g) : g(g) { }
      template <class Iter>
      const edge_descriptor& dereference(const Iter& x) const {
        return g->edge_list[g->trace[x.base()]]; // ?
      }
      const compact_graph* g;
    };
  public:
    typedef boost::counting_iterator_generator<vertex_descriptor>::type
      vertex_iterator;

    typedef boost::iterator_adaptor<std::size_t, out_edge_iter_policies,
      edge_descriptor, const edge_descriptor&, const edge_descriptor*,
      std::bidirectional_iterator_tag, std::ptrdiff_t> out_edge_iterator;

    typedef boost::iterator_adaptor<std::size_t, in_edge_iter_policies,
      edge_descriptor, const edge_descriptor&, const edge_descriptor*,
      std::bidirectional_iterator_tag, std::ptrdiff_t> in_edge_iterator;
    
    typedef typename boost::adjacency_iterator_generator<
      compact_graph, vertex_descriptor, out_edge_iterator>::type
      adjacency_iterator;

    typedef typename EdgeList::const_iterator edge_iterator;
    
    template <typename EdgeIterator>
    compact_graph(EdgeIterator first, EdgeIterator last,
                  vertices_size_type n, edges_size_type m = 0)
      : EdgeBase(first, last, n, m), VertexBase(n) { }

    template <typename EdgeIterator, typename EdgePropertyIterator>
    compact_graph(EdgeIterator first, EdgeIterator last,
                  EdgePropertyIterator ep_iter,
                  vertices_size_type n, edges_size_type m = 0)
      : EdgeBase(first, last, ep_iter, n, m), VertexBase(n) { }
  };

  //===========================================================================
  // Graph traversal functions

  // source() and target() covered by defs in boost/graph/graph_traits.hpp

  template <typename D, typename VP, typename EP>
  std::pair<typename compact_graph<D, VP, EP>::out_edge_iterator,
            typename compact_graph<D, VP, EP>::out_edge_iterator>
  out_edges(std::size_t u, const compact_graph<D, VP, EP>& g)
  {
    typedef typename compact_graph<D, VP, EP>::out_edge_iterator iter;
    typename compact_graph<D, VP, EP>::out_edge_iter_policies p(&g);
    return std::make_pair(iter(g.point[u], p), iter(g.point[u+1], p));
  }

  template <typename D, typename VP, typename EP>
  std::size_t
  out_degree(std::size_t u, const compact_graph<D, VP, EP>& g)
  {
    return g.point[u+1] - g.point[u];
  }

  template <typename D, typename VP, typename EP>
  std::pair<typename compact_graph<D, VP, EP>::in_edge_iterator,
            typename compact_graph<D, VP, EP>::in_edge_iterator>
  in_edges(std::size_t v, const compact_graph<D, VP, EP>& g)
  {
    typedef typename compact_graph<D, VP, EP>::in_edge_iterator iter;
    typename compact_graph<D, VP, EP>::in_edge_iter_policies p(&g);
    return std::make_pair(iter(g.rev_point[v], p), iter(g.rev_point[v+1], p));
  }

  template <typename D, typename VP, typename EP>
  std::size_t
  in_degree(std::size_t u, const compact_graph<D, VP, EP>& g)
  {
    return g.rev_point[u+1] - g.rev_point[u];
  }

  template <typename D, typename VP, typename EP>
  std::pair<typename compact_graph<D, VP, EP>::adjacency_iterator,
            typename compact_graph<D, VP, EP>::adjacency_iterator>
  adjacent_vertices(std::size_t u, const compact_graph<D, VP, EP>& g)
  {
    typedef typename compact_graph<D, VP, EP>::adjacency_iterator adj_iter;
    typename compact_graph<D, VP, EP>::out_edge_iterator first, last;

    boost::tie(first, last) = out_edges(u, g);
    return std::make_pair(adj_iter(first, &g), adj_iter(last, &g));
  }

  template <typename D, typename VP, typename EP>
  std::size_t
  num_vertices(const compact_graph<D, VP, EP>& g)
  {
    return g.point.size() - 1;
  }

  template <typename D, typename VP, typename EP>
  std::pair<typename compact_graph<D, VP, EP>::vertex_iterator,
            typename compact_graph<D, VP, EP>::vertex_iterator>
  vertices(const compact_graph<D, VP, EP>& g)
  {
    return std::make_pair(boost::make_counting_iterator(std::size_t(0)),
                          boost::make_counting_iterator(num_vertices(g)));
  }
  
  template <typename D, typename VP, typename EP>
  std::size_t
  num_edges(const compact_graph<D, VP, EP>& g)
  {
    return g.edge_list.size();
  }
  
  template <typename D, typename VP, typename EP>
  std::pair<typename compact_graph<D, VP, EP>::edge_iterator,
            typename compact_graph<D, VP, EP>::edge_iterator>
  edges(const compact_graph<D, VP, EP>& g)
  {
    typedef typename compact_graph<D, VP, EP>::edge_iterator edge_iter;
    return std::make_pair(g.edge_list.begin(), g.edge_list.end());
  }

  //===========================================================================
  // Property access functions

  
  

} // namespace boost

#endif // BOOST_COMPACT_GRAPH_HPP
