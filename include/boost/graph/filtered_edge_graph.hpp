#ifndef BOOST_FILTERED_EDGE_GRAPH_HPP
#define BOOST_FILTERED_EDGE_GRAPH_HPP

#include <boost/pending/iterator_adaptors.hpp>
#include <boost/graph/detail/adjacency_iterator.hpp>

namespace boost {

  template <class Graph, class Predicate>
  class filtered_edge_graph {
    typedef graph_traits<Graph> Traits;
    typedef filtered_edge_graph self;
  public:
    // constructor
    filtered_edge_graph(Graph& g, Predicate p)
      : m_g(g), m_p(p) { }

    // Graph requirements
    typedef typename Traits::vertex_descriptor          vertex_descriptor;
    typedef typename Traits::edge_descriptor            edge_descriptor;
    typedef typename Traits::directed_category          directed_category;
    typedef typename Traits::edge_parallel_category     edge_parallel_category;

    // IncidenceGraph requirements
    typedef typename filter_iterator<Predicate,
      typename Traits::out_edge_iterator,
      boost::iterator<multi_pass_input_iterator_tag, 
        edge_descriptor, std::ptrdiff_t, 
        edge_descriptor*, edge_descriptor> >::type     out_edge_iterator;
    typedef typename Traits::degree_size_type          degree_size_type;

    // AdjacencyGraph requirements
    typedef typename detail::adjacency_iterator<
       self, vertex_descriptor, out_edge_iterator, 
       out_edge_iterator>::type                        adjacency_iterator;

    // VertexListGraph requirements
    typedef typename Traits::vertex_iterator           vertex_iterator;
    typedef typename Traits::vertices_size_type        vertices_size_type;

    // EdgeListGraph requirements
    typedef typename filter_iterator<Predicate,
      typename Traits::edge_iterator,
      boost::iterator<multi_pass_input_iterator_tag, 
        edge_descriptor, std::ptrdiff_t,
        edge_descriptor*, edge_descriptor> >::type     edge_iterator;
    typedef typename Traits::edges_size_type edges_size_type;

    typedef void in_edge_iterator;

    typedef typename Graph::edge_property_type edge_property_type;
    typedef typename Graph::vertex_property_type vertex_property_type;
    typedef typename Graph::graph_tag graph_tag;

    //private:
    Graph& m_g;
    Predicate m_p;

    typedef filter_iterator_policies<Predicate, 
      typename Traits::out_edge_iterator> out_edge_iter_policy;
    typedef filter_iterator_policies<Predicate, 
      typename Traits::edge_iterator> edge_iter_policy;
  };

  // Helper function
  template <class Graph, class Predicate>
  inline filtered_edge_graph<Graph, Predicate>
  make_filtered_edge_graph(Graph& g, Predicate p) {
    return filtered_edge_graph<Graph, Predicate>(g, p);
  }

  template <class G, class P>
  std::pair<typename graph_traits<G>::vertex_iterator,
            typename graph_traits<G>::vertex_iterator>
  vertices(const filtered_edge_graph<G, P>& g) {
    return vertices(g.m_g);
  }

  template <class G, class P>
  std::pair<typename filtered_edge_graph<G, P>::edge_iterator,
            typename filtered_edge_graph<G, P>::edge_iterator>
  edges(const filtered_edge_graph<G, P>& g) {
    typedef filtered_edge_graph<G, P> Graph;
    typedef typename Graph::edge_iter_policy Pol;
    typename graph_traits<G>::edge_iterator f, l;
    tie(f, l) = edges(g.m_g);
    typedef typename Graph::edge_iterator iter;
    return std::make_pair(iter(f, Pol(g.m_p, l)), iter(l, Pol(g.m_p, l)));
  }

  template <class G, class P>  
  typename graph_traits<G>::vertices_size_type
  num_vertices(const filtered_edge_graph<G, P>& g) {
    return num_vertices(g);
  }

  template <class G, class P>  
  typename graph_traits<G>::edges_size_type
  num_edges(const filtered_edge_graph<G, P>& g) {
    return num_edges(g);
  }
  
  template <class G, class P>
  typename graph_traits<G>::vertex_descriptor
  source(typename graph_traits<G>::edge_descriptor e,
         const filtered_edge_graph<G, P>& g)
  {
    return source(e, g.m_g);
  }

  template <class G, class P>
  typename graph_traits<G>::vertex_descriptor
  target(typename graph_traits<G>::edge_descriptor e,
         const filtered_edge_graph<G, P>& g)
  {
    return target(e, g.m_g);
  }

  template <class G, class P>
  std::pair<typename filtered_edge_graph<G, P>::out_edge_iterator,
            typename filtered_edge_graph<G, P>::out_edge_iterator>
  out_edges(typename graph_traits<G>::vertex_descriptor u,
            const filtered_edge_graph<G, P>& g)
  {
    typedef filtered_edge_graph<G, P> Graph;
    typedef typename Graph::out_edge_iter_policy Pol;
    typedef typename Graph::out_edge_iterator iter;
    typename graph_traits<G>::out_edge_iterator f, l;
    tie(f, l) = out_edges(u, g.m_g);
    return std::make_pair(iter(f, Pol(g.m_p, l)), iter(l, Pol(g.m_p, l)));
  }

  template <class G, class P>
  typename graph_traits<G>::degree_size_type
  out_degree(typename graph_traits<G>::vertex_descriptor u,
             const filtered_edge_graph<G, P>& g)
  {
    typename graph_traits<G>::degree_size_type n = 0;
    typename filtered_edge_graph<G, P>::out_edge_iterator f, l;
    for (tie(f, l) = out_edges(u, g); f != l; ++f)
      ++n;
    return n;
  }

  template <class G, class P>
  std::pair<typename filtered_edge_graph<G, P>::adjacency_iterator,
            typename filtered_edge_graph<G, P>::adjacency_iterator>
  adjacent_vertices(typename graph_traits<G>::vertex_descriptor u,
                    const filtered_edge_graph<G, P>& g)
  {
    typedef filtered_edge_graph<G, P> Graph;
    typedef typename Graph::adjacency_iterator adjacency_iterator;
    typename Graph::out_edge_iterator f, l;
    tie(f, l) = out_edges(u, g);
    return std::make_pair(adjacency_iterator(f, const_cast<Graph*>(&g)),
                          adjacency_iterator(l, const_cast<Graph*>(&g)));
  }
  
  template <class G, class P, class Property>
  typename property_map<G, Property>::type
  get(Property p, filtered_edge_graph<G,P>& g)
  {
    return get(p, g.m_g);
  }

  template <class G, class P,class Property>
  typename property_map<G, Property>::const_type
  get(Property p, const filtered_edge_graph<G,P>& g)
  {
    return get(p, g.m_g);
  }

  template <class G, class P, class Property, class Key>
  typename property_traits<
    typename property_map<G, Property>::const_type
  >::value_type
  get(Property p, const filtered_edge_graph<G,P>& g, const Key& k)
  {
    return get(p, g.m_g, k);
  }

  template <class G, class P, class Property, class Key, class Value>
  void
  put(Property p, const filtered_edge_graph<G,P>& g, const Key& k,
      const Value& val)
  {
    put(p, g.m_g, k, val);
  }

} // namespace boost


#endif // BOOST_FILTERED_EDGE_GRAPH_HPP
