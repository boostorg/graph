#ifndef BOOST_SUBGRAPH_HPP
#define BOOST_SUBGRAPH_HPP

// UNDER CONSTRUCTION

#include <boost/config.hpp>
#include <list>
#include <boost/graph/graph_traits.hpp>

namespace boost {

  struct subgraph_tag { };

  enum vertex_parent_t { vertex_parent = 2111 };
  enum edge_parent_edge_t { edge_parent = 2112 };
  enum vertex_children_t { vertex_children = 2113 };
  enum edge_children_t { edge_children = 2114 };

  BOOST_INSTALL_PROPERTY(vertex, parent);
  BOOST_INSTALL_PROPERTY(edge, parent);
  BOOST_INSTALL_PROPERTY(vertex, children);
  BOOST_INSTALL_PROPERTY(edge, children);

  template <typename Graph>
  class subgraph {
    typedef graph_traits<Graph> Traits;
  public:
    // Graph requirements
    typedef typename Traits::vertex_descriptor          vertex_descriptor;
    typedef typename Traits::edge_descriptor            edge_descriptor;
    typedef typename Traits::directed_category          directed_category;
    typedef typename Traits::edge_parallel_category     edge_parallel_category;

    // IncidenceGraph requirements
    typedef typename Traits::out_edge_iterator         out_edge_iterator;
    typedef typename Traits::degree_size_type          degree_size_type;

    // AdjacencyGraph requirements
    typedef typename Traits::adjacency_iterator        adjacency_iterator;

    // VertexListGraph requirements
    typedef typename Traits::vertex_iterator           vertex_iterator;
    typedef typename Traits::vertices_size_type        vertices_size_type;

    // EdgeListGraph requirements
    typedef typename Traits::edge_iterator             edge_iterator;
    typedef typename Traits::edges_size_type           edges_size_type;

    typedef typename Traits::in_edge_iterator          in_edge_iterator;

    typedef typename Graph::edge_property_type         edge_property_type;
    typedef typename Graph::vertex_property_type       vertex_property_type;
    typedef subgraph_tag                               graph_tag;
    
    // Constructors
    subgraph() : m_parent(0) { }

    subgraph(subgraph<Graph>& parent) 
      : m_parent(&parent)
    {
      m_parent->add_child(*this);
    }
    subgraph(vertices_size_type n) 
      : m_graph(n), m_parent(0)
    {
      m_parent->add_child(*this);
    }
    subgraph(vertices_size_type n, subgraph<Graph>& parent) 
      : m_graph(n), m_parent(parent)
    {
      m_parent->add_child(*this);
    }
  private:
    void add_child(Graph* child) {
      m_children.push_back(child);
    }
    Graph m_graph;
    Graph* m_parent;
    std::list<Graph*> m_children;
  };

  //===========================================================================
  // Functions required by the IncidenceGraph concept 

  template <typename G>
  std::pair<typename graph_traits<G>::out_edge_iterator,
            typename graph_traits<G>::out_edge_iterator>
  out_edges(typename graph_traits<G>::vertex_descriptor u,
            const subgraph<G>& g) 
    { return out_edges(u, g.m_graph); }

  template <typename G>
  typename graph_traits<G>::degree_size_type
  out_degree(typename graph_traits<G>::vertex_descriptor u,
             const subgraph<G>& g) 
    { return out_degree(u, g.m_graph); }

  template <typename G>
  typename graph_traits<G>::vertex_descriptor
  source(typename graph_traits<G>::edge_descriptor e,
         const subgraph<G>& g) 
    { return source(e, g.m_graph); }

  template <typename G>
  typename graph_traits<G>::vertex_descriptor
  target(typename graph_traits<G>::edge_descriptor e,
         const subgraph<G>& g) 
    { return target(e, g.m_graph); }

  //===========================================================================
  // Functions required by the AdjacencyGraph concept 

  template <typename G>
  std::pair<typename subgraph<G>::adjacency_iterator,
            typename subgraph<G>::adjacency_iterator>
  adjacenct_vertices(typename subgraph<G>::vertex_descriptor u,
                     const subgraph<G>& g) 
    { return adjacenct_vertices(u, g.m_graph); }

  //===========================================================================
  // Functions required by the VertexListGraph concept 

  template <typename G>
  std::pair<typename subgraph<G>::vertex_iterator,
            typename subgraph<G>::vertex_iterator>
  vertices(const subgraph<G>& g) 
    { return vertices(g.m_graph); }

  template <typename G>
  typename subgraph<G>::vertices_size_type
  num_vertices(const subgraph<G>& g) 
    { return num_vertices(g.m_graph); }

  //===========================================================================
  // Functions required by the EdgeListGraph concept 
  
  template <typename G>
  std::pair<typename subgraph<G>::edge_iterator,
            typename subgraph<G>::edge_iterator>
  edges(const subgraph<G>& g) 
    { return edges(g.m_graph); }

  template <typename G>
  typename subgraph<G>::edges_size_type
  num_edges(const subgraph<G>& g) 
    { return num_edges(g.m_graph); }

  //===========================================================================
  // Functions required by the MutableGraph concept 

  template <typename G>
  std::pair<typename subgraph<G>::edge_descriptor, bool>
  add_edge(typename subgraph<G>::vertex_descriptor u,
           typename subgraph<G>::vertex_descriptor v,
           const typename G::edge_property_type& ep,
           subgraph<G>& g)
  {
    typename subgraph<G>::edge_descriptor e, parent_e;
    bool inserted;
    if (g.m_parent == 0) // this is the root of the subgraph tree
      tie(e, inserted) = add_edge(u, v, ep, g.m_graph);
    else {
      tie(parent_e, inserted) = add_edge(get(vertex_parent, g.m_graph, u), 
                                         get(vertex_parent, g.m_graph, v),
                                         ep,
                                         *g.m_parent);
      tie(e, inserted) = add_edge(u, v, e, g.m_graph);
      get(edge_children, *g.m_parent)
        [parent_e].push_back(std::make_pair(e, this));
    }
    return std::make_pair(e, inserted);
  }

  template <typename G>
  std::pair<typename subgraph<G>::edge_descriptor, bool>
  add_edge(typename subgraph<G>::vertex_descriptor u,
           typename subgraph<G>::vertex_descriptor v,
           subgraph<G>& g)
  {
    typename G::edge_property_type ep;
    add_edge(u, v, ep, g);
  }

  namespace detail {

    template <typename ChildrenVertices>
    void remove_children_edges(const ChildrenVertices& csv,
                               const ChildrenVertices& ctv)
    {
      // Note: the m_children_vertices sequence has to be in the same
      // order as the m_children sequence.
      typename ChildrenVertices::const_iterator 
        si = csv.begin(), ti = ctv.begin();
      for (; si != csv.end(); ++si, ++ti)
        remove_edge((*si).first, (*ti).first, (*si).second);
    }

    template <typename ChildrenEdges>
    void remove_children_edges(const ChildrenEdges& ce)
    {
      // Note: the m_children_vertices sequence has to be in the same
      // order as the m_children sequence.
      typename ChildrenEdges::const_iterator ei = csv.begin();
      for (; ei != ce.end(); ++ei)
        remove_edge((*ei).first, (*ei).second);
    }
    
  } // namespace detail

  template <typename G>
  void
  remove_edge(typename subgraph<G>::vertex_descriptor u,
              typename subgraph<G>::vertex_descriptor v,
              subgraph<G>& g)
  {
    detail::remove_children_edges
      (get(vertex_children, g.m_graph)[u], 
       get(vertex_children, g.m_graph)[v]);
    
    remove_edge(u, v, g.m_graph);

    if (g.m_parent != 0)
      remove_edge(get(vertex_parent, g.m_graph, u),
                  get(vertex_parent, g.m_graph, v),
                  *g.m_parent);
  }
  
  template <typename G>
  void
  remove_edge(typename subgraph<G>::edge_descriptor e,
	      subgraph<G>& g)
  {
    detail::remove_children_edges(get(children_edges, g.m_graph)[e]);
    
    remove_edge(e, g.m_graph);
    
    if (g.m_parent != 0)
      remove_edge(get(edge_parent, g.m_graph, e), *g.m_parent);
  }

  template <typename G>
  typename subgraph<G>::vertex_descriptor
  add_vertex(subgraph<G>& g)
  {
    
  }
  
  
  
} // namespace boost

#endif // BOOST_SUBGRAPH_HPP
