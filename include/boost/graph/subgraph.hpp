//=======================================================================
// Copyright 2001 University of Notre Dame.
// Author: Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================

#ifndef BOOST_SUBGRAPH_HPP
#define BOOST_SUBGRAPH_HPP

// UNDER CONSTRUCTION

#include <boost/config.hpp>
#include <list>
#include <vector>
#include <map>
#include <cassert>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

namespace boost {

  struct subgraph_tag { };

  // Invariants:
  //   - If vertex u is in subgraph g, then u must be in g.parent().
  //   - If edge e is in subgraph e, then e must be in g.parent().
  //   - If edge e=(u,v) is in the root graph, then edge e
  //     is also in any subgraph that contains both vertex u and v.

  // The Graph template parameter must have a vertex_index 
  // and edge_index internal property. It is assumed that
  // the vertex indices are assigned automatically by the
  // graph during a call to add_vertex(). It is not
  // assumed that the edge vertices are assigned automatically,
  // they are explicitly assigned here.

  template <typename Graph>
  class subgraph {
    typedef graph_traits<Graph> Traits;
  public:
    // Graph requirements
    typedef typename Traits::vertex_descriptor         vertex_descriptor;
    typedef typename Traits::edge_descriptor           edge_descriptor;
    typedef typename Traits::directed_category         directed_category;
    typedef typename Traits::edge_parallel_category    edge_parallel_category;

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
    typedef Graph                                      graph_type;
    
    // Constructors

    // Create the main graph, the root of the subgraph tree
    subgraph(vertices_size_type n) 
      : m_graph(n), m_parent(0), m_vertex_membership(n, true),
	m_edge_counter(0)
    { }
    // Create a subgraph
    subgraph(subgraph<Graph>& parent) 
      : m_parent(&parent), 
        m_vertex_membership(num_vertices(parent.m_graph), false),
        m_local_vertex(num_vertices(parent)),
	m_edge_counter(0)
    {
      m_parent->add_child(this);
    }
    // Create a subgraph with the specified vertex set.
    template <typename VertexIterator>
    subgraph(subgraph<Graph>& parent,
             VertexIterator first, VertexIterator last)
      : m_parent(&parent), 
        m_vertex_membership(num_vertices(parent.m_graph), false),
        m_local_vertex(num_vertices(parent)),
	m_edge_counter(0)
    {
      for (; first != last; ++first)
	add_vertex(*first, *this);
    }

    // local <-> global descriptor conversion functions
    vertex_descriptor local_to_global(vertex_descriptor u_local)
    {
      return m_global_vertex[u_local];
    }
    vertex_descriptor global_to_local(vertex_descriptor u_global)
    {
      return m_local_vertex[u_global];
    }
    edge_descriptor local_to_global(edge_descriptor e_local)
    {
      return m_global_edge[get(edge_index, g, u_local)];
    }
    edge_descriptor global_to_local(edge_descriptor e_global)
    {
      return m_local_edge[get(edge_index, this->root(), u_global)];
    }

    // Is vertex u (of the root graph) contained in this subgraph?
    bool contains(vertex_descriptor u_global)
    {
      return m_vertex_membership[u_global];
    }

    // Return the parent graph.
    subgraph& parent() { return *m_parent; }
    
    // Return the root graph of the subgraph tree.
    subgraph& root() {
      if (m_parent == 0)
	return *this;
      else
	return m_parent->root();
    }

    //  private:
    void add_child(subgraph<Graph>* child) { m_children.push_back(child); }

    Graph m_graph;
    subgraph<Graph>* m_parent;
    std::list<subgraph<Graph>*> m_children;
    std::vector<bool> m_vertex_membership;          // global
    std::vector<vertex_descriptor> m_global_vertex; // local -> global
    std::vector<vertex_descriptor> m_local_vertex;  // global -> local

    typedef typename property_map<Graph, edge_index_t>::type EdgeIndexMap;
    typedef typename property_traits<EdgeIndexMap>::value_type edge_index_type;

    std::vector<edge_index_type> m_global_edge;              // local -> global
    std::map<edge_index_type, edge_index_type> m_local_edge; // global -> local
    edge_index_type m_edge_counter; // for generating unique edge indices

    edge_descriptor
    local_add_edge(vertex_descriptor u_local, vertex_descriptor v_local,  
		   edge_descriptor e_global)
    {
      edge_descriptor e_local; 
      bool inserted;
      tie(e_local, inserted) = add_edge(u_local, v_local, m_graph);
      put(edge_index, m_graph, e_local, m_edge_counter++);
      m_global_edge.push_back(get(edge_index, m_graph, e_global));
      m_local_edge[get(edge_index, this->root(), e_global)] 
	= get(edge_index, m_graph, e_local);
      return e_local;
    }

  };

  //===========================================================================
  // Functions special to the Subgraph Class

  template <typename G>
  typename subgraph<G>::vertex_descriptor
  add_vertex(typename subgraph<G>::vertex_descriptor u_global,
             subgraph<G>& g)
  {
    assert(g.m_parent != 0);
    typename subgraph<G>::vertex_descriptor u_local, v_global, uu_global;
    typename subgraph<G>::edge_descriptor e_global;

    u_local = add_vertex(g.m_graph);
    g.m_global_vertex.push_back(u_global);
    g.m_local_vertex[u_global] = u_local;
    g.m_vertex_membership[u_global] = true;
    
    subgraph<G>& r = g.root();

    // remember edge global and local maps
    {
      typename subgraph<G>::out_edge_iterator ei, ei_end;
      for (tie(ei, ei_end) = out_edges(u_global, r); 
	   ei != ei_end; ++ei) {
	e_global = *ei;
	v_global = target(e_global, r);
	if (g.contains(v_global))
	  g.local_add_edge(u_local, g.global_to_local(v_global), e_global);
      }
    }
    {
      typename subgraph<G>::vertex_iterator vi, vi_end;
      typename subgraph<G>::out_edge_iterator ei, ei_end;
      for (tie(vi, vi_end) = vertices(r); vi != vi_end; ++vi) {
	v_global = *vi;
	for (tie(ei, ei_end) = out_edges(*vi, r); ei != ei_end; ++ei) {
	  e_global = *ei;
	  uu_global = target(e_global, r);
	  if (uu_global == u_global && g.contains(v_global))
	    g.local_add_edge(g.global_to_local(v_global), u_local, e_global);
	}
      }
    }

    return u_local;
  }  

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
  adjacent_vertices(typename subgraph<G>::vertex_descriptor u,
                    const subgraph<G>& g) 
    { return adjacent_vertices(u, g.m_graph); }

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

  namespace detail {

    template <typename Vertex, typename Edge, typename Graph>
    void add_edge_recur_down
    (Vertex u_global, Vertex v_global, Edge e_global, subgraph<Graph>& g);

    template <typename Vertex, typename Edge, typename Children, typename G>
    void children_add_edge(Vertex u_global, Vertex v_global, Edge e_global,
                           Children& c, subgraph<G>* orig)
    {
      for (typename Children::iterator i = c.begin(); i != c.end(); ++i)
        if ((*i)->contains(u_global) && (*i)->contains(v_global))
          add_edge_recur_down(u_global, v_global, e_global, **i, orig);
    }

    template <typename Vertex, typename Edge, typename Graph>
    void add_edge_recur_down
      (Vertex u_global, Vertex v_global, Edge e_global, subgraph<Graph>& g,
       subgraph<Graph>* orig)
    {
      if (&g != orig) {
        Vertex u_local, v_local;
        u_local = g.global_to_local(u_global);
        v_local = g.global_to_local(v_global);
	g.local_add_edge(u_local, v_local, e_global);
      }
      children_add_edge(u_global, v_global, e_global, g.m_children, orig);
    }

    template <typename Vertex, typename Graph>
    std::pair<typename subgraph<Graph>::edge_descriptor, bool>
    add_edge_recur_up(Vertex u_global, Vertex v_global,
                      const typename Graph::edge_property_type& ep,
                      subgraph<Graph>& g, subgraph<Graph>* orig)
    {
      if (g.m_parent == 0) {
        assert(g.contains(u_global) && g.contains(v_global));
        typename subgraph<Graph>::edge_descriptor e_global;
        bool inserted;
        tie(e_global, inserted) = add_edge(u_global, v_global, ep, g.m_graph);
	put(edge_index, g.m_graph, e_global, g.m_edge_counter++);
        children_add_edge(u_global, v_global, e_global, g.m_children, orig);
        return std::make_pair(e_global, inserted);
      } else
        return add_edge_recur_up(u_global, v_global, ep, *g.m_parent, orig);
    }

  } // namespace detail

  // Add an edge to the subgraph g, specified by the local vertex
  // descriptors u and v. In addition, the edge will be added to any
  // other subgraphs which contain vertex descriptors u and v.

  template <typename G>
  std::pair<typename subgraph<G>::edge_descriptor, bool>
  add_edge(typename subgraph<G>::vertex_descriptor u_local,
           typename subgraph<G>::vertex_descriptor v_local,
           const typename G::edge_property_type& ep,
           subgraph<G>& g)
  {
    if (g.m_parent == 0) // u_local and v_local are really global
      return detail::add_edge_recur_up(u_local, v_local, ep, g, &g);
    else {
      assert(g.contains(g.local_to_global(u_local))
             && g.contains(g.local_to_global(v_local)));
      typename subgraph<G>::edge_descriptor e_local, e_global;
      bool inserted;
      tie(e_global, inserted) = detail::add_edge_recur_up
        (g.local_to_global(u_local), g.local_to_global(v_local), ep, g, &g);
      e_local = g.local_add_edge(u_local, v_local, e_global);
      return std::make_pair(e_local, inserted);
    }
  }

  template <typename G>
  std::pair<typename subgraph<G>::edge_descriptor, bool>
  add_edge(typename subgraph<G>::vertex_descriptor u,
           typename subgraph<G>::vertex_descriptor v,
           subgraph<G>& g)
  {
    typename G::edge_property_type ep;
    return add_edge(u, v, ep, g);
  }

  namespace detail {

    //-------------------------------------------------------------------------
    // implementation of remove_edge(u,v,g)

    template <typename Vertex, typename Graph>
    void remove_edge_recur_down(Vertex u_global, Vertex v_global, 
                                subgraph<Graph>& g);

    template <typename Vertex, typename Children>
    void children_remove_edge(Vertex u_global, Vertex v_global,
                              const Children& c)
    {
      for (typename Children::iterator i = c.begin(); i != c.end(); ++i)
        if ((*i)->m_vertex_membership[u_global]
            && (*i)->m_vertex_membership[v_global])
          remove_edge_recur_down(u_global, v_global, **i);
    }
    
    template <typename Vertex, typename Graph>
    void remove_edge_recur_down(Vertex u_global, Vertex v_global, 
                                subgraph<Graph>& g)
    {
      Vertex u_local, v_local;
      u_local = g.m_local_vertex[u_global];
      v_local = g.m_local_vertex[v_global];
      remove_edge(u_local, v_local, g.m_graph);
      children_remove_edge(u_global, v_global, g.m_children);
    }

    template <typename Vertex, typename Graph>
    void remove_edge_recur_up(Vertex u_global, Vertex v_global, 
                              subgraph<Graph>& g)
    {
      if (g.m_parent == 0) {
        remove_edge(u_global, v_global, g.m_graph);
        children_remove_edge(u_global, v_global, g.m_children);
      } else
        remove_edge_recur_up(u_global, v_global, *g.m_parent);
    }

    //-------------------------------------------------------------------------
    // implementation of remove_edge(e,g)

    template <typename Edge, typename Graph>
    void remove_edge_recur_down(Edge e_global, subgraph<Graph>& g);

    template <typename Edge, typename Children>
    void children_remove_edge(Edge e_global, const Children& c)
    {
      for (typename Children::iterator i = c.begin(); i != c.end(); ++i)
        if ((*i)->m_vertex_membership[u] && (*i)->m_vertex_membership[v])
          remove_edge_recur_down(u_global, v_global, **i);
    }

    template <typename Edge, typename Graph>
    void remove_edge_recur_down(Edge e_global, subgraph<Graph>& g)
    {
      remove_edge(g.global_to_local(e_global), g.m_graph);
      children_remove_edge(e_global, g.m_children);
    }

    template <typename Edge, typename Graph>
    void remove_edge_recur_up(Edge e_global, subgraph<Graph>& g)
    {
      if (g.m_parent == 0) {
        remove_edge(e_global, g.m_graph);
        children_remove_edge(e_global, g.m_children);
      } else
        remove_edge_recur_up(e_global, *g.m_parent);
    }

  } // namespace detail

  template <typename G>
  void
  remove_edge(typename subgraph<G>::vertex_descriptor u_local,
              typename subgraph<G>::vertex_descriptor v_local,
              subgraph<G>& g)
  {
    assert(g.m_vertex_membership[u_local] && g.m_vertex_membership[v_local]);
    if (g.m_parent == 0)
      detail::remove_edge_recur_up(u_local, v_local, g.m_graph);
    else
      detail::remove_edge_recur_up(g.m_global_vertex[u_local],
                                   g.m_global_vertex[v_local], g);
  }
  
  template <typename G>
  void
  remove_edge(typename subgraph<G>::edge_descriptor e_local,
              subgraph<G>& g)
  {
    if (g.m_parent == 0)
      detail::remove_edge_recur_up(e_local, g.m_graph);
    else
      detail::remove_edge_recur_up(get(edge_global, g.m_graph, e_local), g);
  }

  namespace detail {
    
    template <typename G>
    typename subgraph<G>::vertex_descriptor
    add_vertex_recur_up(subgraph<G>& g)
    {
      typename subgraph<G>::vertex_descriptor u_local, u_global;
      if (g.m_parent == 0) {
        u_global = add_vertex(g.m_graph);       
      } else {
        u_global = add_vertex_recur_up(*g.m_parent);
        u_local = add_vertex(g.m_graph);
        g.m_global_vertex.push_back(u_global);
        g.m_local_vertex[u_global] = u_local;
      }
      g.m_vertex_membership[u_global] = true;
      return u_global;
    }
    
  } // namespace detail

  template <typename G>
  typename subgraph<G>::vertex_descriptor
  add_vertex(subgraph<G>& g)
  {
    typename subgraph<G>::vertex_descriptor  u_local, u_global;
    if (g.m_parent == 0)
      u_global = add_vertex(g.m_graph);
    else {
      u_global = detail::add_vertex_recur_up(g);
      u_local = add_vertex(g.m_graph);
      g.m_global_vertex.push_back(u_global);
      g.m_local_vertex[u_global] = u_local;
    }
    g.m_vertex_membership[u_global] = true;
    return u_local;
  }

  template <typename G>
  void remove_vertex(typename subgraph<G>::vertex_descriptor u,
                     subgraph<G>& g)
  {
    // UNDER CONSTRUCTION
    assert(false);
  }


  //===========================================================================
  // Functions required by the PropertyGraph concept 

  template <typename Graph, typename Property, typename Key>
  class subgraph_property_map 
    : public put_get_at_helper< 
        typename property_traits<
           typename property_map<Graph, Property>::type
         >::value_type,
        subgraph_property_map<Graph, Property, Key> >
  {
    typedef typename property_map<Graph, Property>::type PropertyMap;
  public:
    typedef typename property_traits<PropertyMap>::value_type value_type;
    typedef Key key_type;

    subgraph_property_map() { }

    subgraph_property_map(subgraph<Graph>& g)
      : m_g(&g), m_pmap(get(Property, g.root())) { }
    
    inline value_type& operator[](key_type e_local) {
      if (m_g->m_parent == 0)
	return m_pmap[e_local];
      else
	return m_pmap[m_g->local_to_global(e_local)];
    }
    inline const value_type& operator[](key_type e_local) const {
      if (m_g->m_parent == 0)
	return m_pmap[e_local];
      else
	return m_pmap[m_g->local_to_global(e_local)];
    }
    subgraph<Graph>* m_g;
    ProperyMap m_pmap;
  };

  namespace detail {

    struct subgraph_vertex_property_selector {
      template <class Graph, class Property, class Tag>
      struct bind {
	typedef subgraph_property_map<typename Graph::graph_type, Property,
	  typename Graph::vertex_descriptor> type;
	typedef type const_type;
      };
    };

    struct subgraph_edge_property_selector {
      template <class Graph, class Property, class Tag>
      struct bind {
	typedef subgraph_property_map<typename Graph::graph_type, Property,
	  typename Graph::edge_descriptor> type;
	typedef type const_type;
      };
    };
        
  } // namespace detail

  template <>
  struct vertex_property_selector<subgraph_tag> {
    typedef detail::subgraph_vertex_property_selector type;
  };

  template <>
  struct edge_property_selector<subgraph_tag> {
    typedef detail::subgraph_edge_property_selector type;
  };

  template <typename G, typename Property>
  typename property_map<G, Property>::type
  get(Property, subgraph<G>& g)
  {
    typedef typename property_map<G, Property>::type PMap;
    return PMap(g);
  }

  template <typename G, typename P,typename Property>
  typename property_map<G, Property>::const_type
  get(Property, const subgraph<G>& g)
  {
    typedef typename property_map<G, Property>::const_type PMap;
    return PMap(g);
  }

  template <typename G, typename Property, typename Key>
  typename property_traits<
    typename property_map<G, Property>::const_type
  >::value_type
  get(Property, const subgraph<G>& g, const Key& k)
  {
    typedef typename property_map<G, Property>::const_type PMap;
    PMap pmap(g);
    return pmap[k];
  }

  template <typename G, typename Property, typename Key, typename Value>
  void
  put(Property, const subgraph<G>& g, const Key& k, const Value& val)
  {
    typedef typename property_map<G, Property>::const_type PMap;
    PMap pmap(g);
    pmap[k] = val;
  }

  
} // namespace boost

#endif // BOOST_SUBGRAPH_HPP
