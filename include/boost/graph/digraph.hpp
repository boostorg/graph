// Copyright Jeremy Siek 2004

// This file was an experiment to try supporting operator-> to access
// vertex and edge properties from a vertex or descriptor.  The
// experiment was abandoned because of const issues.

#ifndef BOOST_GRAPH_DIGRAPH_HPP
#define BOOST_GRAPH_DIGRAPH_HPP

#include <vector>
#include <list>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/graph/graph_traits.hpp>

namespace boost {

  template <class Vertex, class Edge>
  class digraph
  {
  public:
    template <class V>
    class vertex_desc {
    public:
      vertex_desc() { }
      template <class U>
      vertex_desc(const vertex_desc<U>& u) : m_index(u.m_index), m_property(u.m_property) { }
      vertex_desc(std::size_t i, V* prop) 
        : m_index(i), m_property(prop) { }
      bool operator==(const vertex_descriptor& u) const
          { return m_property == u.m_property; }
      bool operator!=(const vertex_descriptor& u) const
          { return m_property != u.m_property; }
      V* operator->() const { return m_property; }
      std::size_t index() const { return m_index; }
      operator std::size_t() const { return m_index; }
    private:
      std::size_t m_index;
      V* m_property;
    };
    typedef vertex_desc<Vertex*> vertex_descriptor;
    typedef vertex_desc<const Vertex*> const_vertex_descriptor;

    template <class E>
    class edge_desc {
    public:
      edge_desc() { }
      template <class F>
      edge_desc(const edge_desc<F>& e) : m_source(e.m_source), m_target(e.m_target), m_property(e.m_property) { }
      edge_desc(std::size_t u, std::size_t v, E* prop)
        : m_source(u), m_target(v), m_property(prop) { }
      bool operator==(const edge_descriptor& e) const
          { return m_property == e.m_property; }
      bool operator!=(const edge_descriptor& e) const
          { return m_property != e.m_property; }
      E* operator->() const { return m_property; }
      std::size_t m_source;
      std::size_t m_target;
      E* m_property;
    };
    typedef edge_desc<E*> edge_descriptor;
    typedef edge_desc<const E*> const_edge_descriptor;

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
                                edge_descriptor, use_default, edge_descriptor>
    {
      typedef iterator_adaptor<out_edge_iterator,
                                typename std::list<edge_info>::const_iterator,
                               edge_descriptor, use_default, edge_descriptor> super_t;
    public:
      out_edge_iterator(typename std::list<edge_info>::const_iterator i, std::size_t src)
        : super_t(i), m_src(src) { }
      edge_descriptor operator*() const {
        return edge_descriptor(m_src, base()->m_target, const_cast<Edge*>(&base()->m_property));
      }
    private:
      std::size_t m_src;
    };
    class adjacency_iterator
      : public iterator_adaptor<adjacency_iterator,
                                typename std::list<edge_info>::const_iterator,
                                vertex_descriptor, use_default, vertex_descriptor>
    {
      typedef iterator_adaptor<adjacency_iterator,
                                typename std::list<edge_info>::const_iterator,
                               vertex_descriptor, use_default, vertex_descriptor> super_t;
    public:
      adjacency_iterator(typename std::list<edge_info>::const_iterator i, std::size_t src, typename std::vector<vertex_info>::const_iterator verts)
        : super_t(i), m_src(src),  m_verts(verts) { }
      vertex_descriptor operator*() const {
        return vertex_descriptor(base()->m_target, const_cast<Vertex*>(&m_verts[base()->m_target].m_property));
      }
    private:
      std::size_t m_src;
      typename std::vector<vertex_info>::const_iterator m_verts;
    };
    typedef void in_edge_iterator;
    typedef void vertex_iterator;
    typedef void edge_iterator;
    typedef directed_tag directed_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
    struct traversal_category : public incidence_graph_tag, adjacency_graph_tag { };
    typedef std::size_t vertices_size_type;
    typedef std::size_t edges_size_type;
    typedef std::size_t degree_size_type;

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
  typename digraph<V,E>::vertex_descriptor
  source(typename digraph<V,E>::edge_descriptor e,
         const digraph<V,E>& g)
  {
    typedef typename digraph<V,E>::vertex_descriptor VD;
    return VD(e.m_source, const_cast<V*>(&g.m_adj_list[e.m_source].m_property));
  }

  template <class V, class E>
  typename digraph<V,E>::vertex_descriptor
  target(typename digraph<V,E>::edge_descriptor e,
         const digraph<V,E>& g)
  {
    typedef typename digraph<V,E>::vertex_descriptor VD;
    return VD(e.m_target, const_cast<V*>(&g.m_adj_list[e.m_target].m_property));
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
    typedef typename digraph<V,E>::out_edge_iterator OEI;
    OEI first(g.m_adj_list[u].m_out_edges.begin(), u),
      last(g.m_adj_list[u].m_out_edges.end(), u);
    return std::make_pair(first, last);
  }
  template <class V, class E>
  std::size_t
  out_degree(typename digraph<V,E>::vertex_descriptor u,
             const digraph<V,E>& g)
  {
    return g.m_adj_list[u].m_out_edges.size();
  }

  template <class V, class E>
  std::pair<typename digraph<V,E>::adjacency_iterator,
            typename digraph<V,E>::adjacency_iterator>
  adjacent_vertices(typename digraph<V,E>::vertex_descriptor u,
                    const digraph<V,E>& g)
  {
    typedef typename digraph<V,E>::adjacency_iterator AI;
    AI first(g.m_adj_list[u].m_out_edges.begin(), u, g.m_adj_list.begin()),
      last(g.m_adj_list[u].m_out_edges.end(), u, g.m_adj_list.begin());
    return std::make_pair(first, last);
  }

} // namespace boost

#endif // BOOST_GRAPH_DIGRAPH_HPP
