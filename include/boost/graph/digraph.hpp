#ifndef BOOST_GRAPH_DIGRAPH_HPP
#define BOOST_GRAPH_DIGRAPH_HPP

#include <vector>
#include <list>
#include <boost/iterator/iterator_adaptor.hpp>

namespace boost {

  template <class Vertex, class Edge>
  class digraph
  {
  public:
    class vertex_descriptor {
    public:
      vertex_descriptor(std::size_t i, Vertex* prop) 
        : m_property(prop) { }
      Vertex* operator->() const { return m_property; }
      std::size_t index() const { return m_index; }
      // or perhaps
      operator std::size_t() const { return m_index; }
    private:
      std::size_t m_index;
      Vertex* m_property;
    };
    class edge_descriptor {
    public:
      edge_descriptor(std::size_t u, std::size_t v, Edge* prop)
        : m_source(u), m_target(v), m_property(prop) { }
      Edge* operator->() const { return m_property; }
      std::size_t m_source;
      std::size_t m_target;
      Edge* m_property;
    };
    //private: would be private if friends worked -JGS
    struct edge_info {
      edge_info(const Edge& e) : m_property(e) { }
      std::size_t m_target;
      Edge m_property;
    };
    struct vertex_info {
      vertex_info(std::size_t i, const Vertex& v)
        : m_index(i), m_property(v) { }
      std::size_t m_index;
      Vertex m_property;
      std::list<edge_info> m_out_edges;
    };
    class out_edge_iterator
      : public iterator_adaptor<out_edge_iterator,
                                typename std::list<edge_info>::const_iterator,
                                edge_descriptor, edge_descriptor>
    {
    public:
      edge_descriptor operator*() const {
        return edge_descriptor(m_src, base()->m_target, &base()->m_property);
      }
    private:
      std::size_t m_src;
    };

    std::vector<vertex_info> m_adj_list;
  };

  template <class V, class E>
  typename digraph<V,E>::vertex_descriptor
  add_vertex(const V& vertex, digraph<V,E>& g)
  {
    typedef typename digraph<V,E>::vertex_info VI;
    std::size_t i = g.m_adj_list.size();
    VI vi(i, vertex);
    g.m_adj_list.push_back(vi);
    typedef typename digraph<V,E>::vertex_descriptor VD;
    return VD(i, &g.m_adj_list.back().m_property);
  }
  
  template <class V, class E>
  std::pair<typename digraph<V,E>::edge_descriptor, bool>
  edge(typename digraph<V,E>::vertex_descriptor u,
       typename digraph<V,E>::vertex_descriptor v,
       const digraph<V,E>& g)
  {
    typedef typename digraph<V,E>::edge_descriptor ED;
    typedef typename digraph<V,E>::edge_info EI;
    for (typename std::list<EI>::const_iterator i = g.m_adj_list[u].begin(); i != g.m_adj_list[u].end(); ++i)
      if (i->m_target == v.index())
        return std::make_pair(ED(u, v, &i->m_property), true);
    return std::make_pair(ED(u, v, 0), false);
  }

  template <class V, class E>
  std::pair<typename digraph<V,E>::out_edge_iterator,
            typename digraph<V,E>::out_edge_iterator>
  out_edges(typename digraph<V,E>::vertex_descriptor u,
            const digraph<V,E>& g)
  {
    
  }


} // namespace boost

#endif // BOOST_GRAPH_DIGRAPH_HPP
