#ifndef BOOST_GRAPH_ARCHETYPES_HPP
#define BOOST_GRAPH_ARCHETYPES_HPP

#include <boost/property_map.hpp>
#include <boost/concept_archetype.hpp>

namespace boost { // should use a different namespace for this
  
  //===========================================================================
  template <typename Vertex, typename Directed, typename ParallelCategory>
  struct incidence_graph_archetype
  {
    typedef incidence_graph_tag traversal_category;
#if 0
    typedef immutable_graph_tag mutability_category;
#endif
    typedef Vertex vertex_descriptor;
    typedef unsigned int degree_size_type;
    typedef unsigned int vertices_size_type;
    typedef unsigned int edges_size_type;
    struct edge_descriptor {
      edge_descriptor() { }
      edge_descriptor(const detail::dummy_constructor&) { }
      bool operator==(const edge_descriptor&) const { return false; }
      bool operator!=(const edge_descriptor&) const { return false; }
    };
    typedef input_iterator_archetype<edge_descriptor> out_edge_iterator;

    typedef Directed directed_category;
    typedef ParallelCategory edge_parallel_category;

    typedef void adjacency_iterator;
    typedef void in_edge_iterator;
    typedef void vertex_iterator;
    typedef void edge_iterator;
  };
  template <typename V, typename D, typename P>
  V source(const typename incidence_graph_archetype<V, D, P>::edge_descriptor&,
	   const incidence_graph_archetype<V,D,P>& )
  {
    return V(dummy_cons);
  }
  template <typename V, typename D, typename P>
  V target(const typename incidence_graph_archetype<V, D, P>::edge_descriptor&,
	   const incidence_graph_archetype<V,D,P>& )
  {
    return V(dummy_cons);
  }
  
  template <typename V, typename D, typename P>
  std::pair<typename incidence_graph_archetype<V,D,P>::out_edge_iterator,
            typename incidence_graph_archetype<V,D,P>::out_edge_iterator>
  out_edges(const V&, const incidence_graph_archetype<V,D,P>& )
  {
    typedef typename incidence_graph_archetype<V,D,P>::out_edge_iterator Iter;
    return std::make_pair(Iter(), Iter());
  }
  
  template <typename V, typename D, typename P>
  typename incidence_graph_archetype<V,D,P>::degree_size_type
  out_degree(const V&, const incidence_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  template <typename V, typename D, typename P>
  typename incidence_graph_archetype<V,D,P>::vertices_size_type
  num_vertices(const incidence_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  template <typename V, typename D, typename P>
  typename incidence_graph_archetype<V,D,P>::edges_size_type
  num_edges(const incidence_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  //===========================================================================
  template <typename Vertex, typename Directed, typename ParallelCategory>
  struct adjacency_graph_archetype
  {
    typedef adjacency_graph_tag traversal_category;
#if 0
    typedef immutable_graph_tag mutability_category;
#endif
    typedef Vertex vertex_descriptor;
    typedef unsigned int degree_size_type;
    typedef unsigned int vertices_size_type;
    typedef unsigned int edges_size_type;
    typedef void edge_descriptor;
    typedef input_iterator_archetype<Vertex> adjacency_iterator;

    typedef Directed directed_category;
    typedef ParallelCategory edge_parallel_category;

    typedef void in_edge_iterator;
    typedef void out_edge_iterator;
    typedef void vertex_iterator;
    typedef void edge_iterator;
  };
  
  template <typename V, typename D, typename P>
  std::pair<typename adjacency_graph_archetype<V,D,P>::adjacency_iterator,
            typename adjacency_graph_archetype<V,D,P>::adjacency_iterator>
  adjacent_vertices(const V&, const adjacency_graph_archetype<V,D,P>& )
  {
    typedef typename adjacency_graph_archetype<V,D,P>::adjacency_iterator Iter;
    return std::make_pair(Iter(), Iter());
  }

  template <typename V, typename D, typename P>
  typename adjacency_graph_archetype<V,D,P>::degree_size_type
  out_degree(const V&, const adjacency_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  template <typename V, typename D, typename P>
  typename adjacency_graph_archetype<V,D,P>::vertices_size_type
  num_vertices(const adjacency_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  template <typename V, typename D, typename P>
  typename adjacency_graph_archetype<V,D,P>::edges_size_type
  num_edges(const adjacency_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  //===========================================================================
  template <typename Vertex, typename Directed, typename ParallelCategory>
  struct vertex_list_graph_archetype
    : public incidence_graph_archetype<Vertex, Directed, ParallelCategory>,
      public adjacency_graph_archetype<Vertex, Directed, ParallelCategory>
  {
    typedef incidence_graph_archetype<Vertex, Directed, ParallelCategory> Incidence;
    typedef adjacency_graph_archetype<Vertex, Directed, ParallelCategory> Adjacency;

    typedef vertex_list_graph_tag traversal_category;
#if 0
    typedef immutable_graph_tag mutability_category;
#endif
    typedef Vertex vertex_descriptor;
    typedef unsigned int degree_size_type;
    typedef typename Incidence::edge_descriptor edge_descriptor;
    typedef typename Incidence::out_edge_iterator out_edge_iterator;
    typedef typename Adjacency::adjacency_iterator adjacency_iterator;

    typedef input_iterator_archetype<Vertex> vertex_iterator;
    typedef unsigned int vertices_size_type;
    typedef unsigned int edges_size_type;

    typedef Directed directed_category;
    typedef ParallelCategory edge_parallel_category;

    typedef void in_edge_iterator;
    typedef void edge_iterator;
  };
  
  template <typename V, typename D, typename P>
  std::pair<typename vertex_list_graph_archetype<V,D,P>::vertex_iterator,
            typename vertex_list_graph_archetype<V,D,P>::vertex_iterator>
  vertices(const vertex_list_graph_archetype<V,D,P>& )
  {
    typedef typename vertex_list_graph_archetype<V,D,P>::vertex_iterator Iter;
    return std::make_pair(Iter(), Iter());
  }

  template <typename V, typename D, typename P>
  typename vertex_list_graph_archetype<V,D,P>::vertices_size_type
  num_vertices(const vertex_list_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  template <typename V, typename D, typename P>
  typename vertex_list_graph_archetype<V,D,P>::edges_size_type
  num_edges(const vertex_list_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  // ambiguously inherited from incidence graph and adjacency graph
  template <typename V, typename D, typename P>
  typename vertex_list_graph_archetype<V,D,P>::degree_size_type
  out_degree(const V&, const vertex_list_graph_archetype<V,D,P>& )
  {
    return 0;
  }

  //===========================================================================

  struct property_graph_archetype_tag { };

  template <typename GraphArchetype, typename Property, typename ValueArch>
  struct property_graph_archetype : public GraphArchetype
  {
    typedef property_graph_archetype_tag graph_tag;
    typedef ValueArch vertex_property_type;
    typedef ValueArch edge_property_type;
  };

  struct choose_edge_property_map_archetype {
    template <typename Graph, typename Property, typename Tag>
    struct bind {
      typedef mutable_lvalue_property_map_archetype
        <typename Graph::edge_descriptor, Property> type;
      typedef lvalue_property_map_archetype
        <typename Graph::edge_descriptor, Property> const_type;
    };
  };
  template <>
  struct edge_property_selector<property_graph_archetype_tag> {
    typedef choose_edge_property_map_archetype type;
  };
  
  struct choose_vertex_property_map_archetype {
    template <typename Graph, typename Property, typename Tag>
    struct bind {
      typedef mutable_lvalue_property_map_archetype
	<typename Graph::vertex_descriptor, Property> type;
      typedef lvalue_property_map_archetype
	<typename Graph::vertex_descriptor, Property> const_type;
    };
  };

  template <>
  struct vertex_property_selector<property_graph_archetype_tag> {
    typedef choose_vertex_property_map_archetype type;
  };
  
  template <typename G, typename P, typename V>
  typename property_map<property_graph_archetype<G, P, V>, P>::type
  get(P, property_graph_archetype<G, P, V>&) {
    return typename property_map<property_graph_archetype<G, P, V>, P>::type();
  }

  template <typename G, typename P, typename V>
  typename property_map<property_graph_archetype<G, P, V>, P>::const_type
  get(P, const property_graph_archetype<G, P, V>&) {
    return typename property_map<property_graph_archetype<G, P, V>, P>::const_type();
  }

  template <typename G, typename P, typename K, typename V>
  typename property_traits<typename property_map<property_graph_archetype<G, P, V>, P>::const_type>::value_type
  get(P p, const property_graph_archetype<G, P, V>& g, K k) {
    return get( get(p, g), k);
  }

  template <typename G, typename P, typename V, typename Key>
  void
  put(P p, property_graph_archetype<G, P, V>& g, 
      const Key& key, const V& value)
  {
    typedef typename boost::property_map<property_graph_archetype<G, P, V>, P>::type Map;
    Map pmap = get(p, g);
    put(pmap, key, value);
  }

  struct color_value_archetype {
    color_value_archetype() { }
    color_value_archetype(detail::dummy_constructor) { }
    bool operator==(const color_value_archetype& ) const { return true; }
    bool operator!=(const color_value_archetype& ) const { return true; }
  };
  template <>
  struct color_traits<color_value_archetype> {
    static color_value_archetype white()
      { return color_value_archetype(dummy_cons); }
    static color_value_archetype gray()
      { return color_value_archetype(dummy_cons); }
    static color_value_archetype black()
      { return color_value_archetype(dummy_cons); }
  };

  template <typename T>
  class buffer_archetype {
  public:
    void push(const T&) {}
    void pop() {}
    T& top() { return s_top; }
    const T& top() const { return s_top; }
    bool empty() const { return true; }
  private:
    static T s_top;
  };
  template <typename T>
  T buffer_archetype<T>::s_top(dummy_cons);
  
} // namespace boost


#endif // BOOST_GRAPH_ARCHETYPES_HPP
