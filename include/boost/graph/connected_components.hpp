//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
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
//
#ifndef BOOST_GRAPH_CONNECTED_COMPONENTS_HPP
#define BOOST_GRAPH_CONNECTED_COMPONENTS_HPP

#include <list>
#include <deque>
#include <queue>
#include <stack>
#include <algorithm>

#include <boost/config.hpp>

#include <boost/utility.hpp> // for tie()
#include <boost/pending/disjoint_sets.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/transpose_graph.hpp>
#include <boost/graph/properties.hpp>
#include <boost/pending/indirect_cmp.hpp>

namespace boost {

  // We use this visitor to record which components a vertex is
  // attributed to during the second DFS traversal in the strongly
  // connected components algorithm.
  template <class ComponentsPA, class DFSVisitor>
  class components_recorder : public DFSVisitor
  {
    typedef typename property_traits<ComponentsPA>::value_type comp_type;
  public:
    components_recorder(ComponentsPA c, 
			comp_type& c_count, 
			DFSVisitor v)
      : DFSVisitor(v), m_component(c), m_count(c_count) {}

    template <class Vertex, class Graph>
    void start_vertex(Vertex u, Graph& g) {
      ++m_count;
      DFSVisitor::start_vertex(u, g);
    }
    template <class Vertex, class Graph>
    void discover_vertex(Vertex u, Graph& g) {
      put(m_component, u, m_count);
      DFSVisitor::discover_vertex(u, g);
    }
  protected:
    ComponentsPA m_component;
    comp_type& m_count;
  };

  template <class DiscoverTimePA, class FinishTimePA, class TimeT, 
    class DFSVisitor>
  class time_recorder : public DFSVisitor
  {
  public:
    time_recorder(DiscoverTimePA d, FinishTimePA f, TimeT& t, DFSVisitor v)
      : DFSVisitor(v), m_discover_time(d), m_finish_time(f), m_t(t) {}

    template <class Vertex, class Graph>
    void discover_vertex(Vertex u, Graph& g) {
      put(m_discover_time, u, ++m_t);
      DFSVisitor::discover_vertex(u, g);
    }
    template <class Vertex, class Graph>
    void finish_vertex(Vertex u, Graph& g) {
      put(m_finish_time, u, ++m_t);
      DFSVisitor::discover_vertex(u, g);
    }
  protected:
    DiscoverTimePA m_discover_time;
    FinishTimePA m_finish_time;
    TimeT m_t;
  };
  template <class DiscoverTimePA, class FinishTimePA, class TimeT, 
    class DFSVisitor>
  time_recorder<DiscoverTimePA, FinishTimePA, TimeT, DFSVisitor>
  record_times(DiscoverTimePA d, FinishTimePA f, TimeT& t, DFSVisitor vis) {
    return time_recorder<DiscoverTimePA, FinishTimePA, TimeT, DFSVisitor>
      (d, f, t, vis);
  }

  namespace detail {

    /* an undirected graph */
    template < class Graph, class DFSVisitor, class Components, class Color >
    inline typename property_traits<Components>::value_type
    connected_components(Graph& g, DFSVisitor v, 
			 Components c, Color color, undirected_tag) {
      typedef typename property_traits<Components>::value_type
	count_type;
      count_type c_count(-1); // we want first component to be '0', and the increment happens first

      typedef components_recorder<Components, DFSVisitor> ComponentRecorder;
      ComponentRecorder vis(c, c_count, v);

      depth_first_search(g, vis, color);

      return c_count + 1;
    }

    /* a directed graph */
    template <class Graph, class DFSVisitor, class Components,
	      class DiscoverTime, class FinishTime,
	      class Color>
    inline typename property_traits<Components>::value_type
    connected_components(Graph& G, DFSVisitor v, Components c, DiscoverTime d,
			 FinishTime f, Color color, directed_tag)
    {
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typename property_traits<Color>::value_type cc = get(color, Vertex());
      int time = 0;
      depth_first_search(G, record_times(d, f, time, v), color);

      Graph G_T(num_vertices(G));
      transpose_graph(G, G_T);

      typedef typename property_traits<Components>::value_type count_type;

      count_type c_count(0);
      components_recorder<Components, dfs_visitor<> > 
	vis(c, c_count, dfs_visitor<>());

      // initialize G_T
      typename graph_traits<Graph>::vertex_iterator ui, ui_end;
      for (tie(ui, ui_end) = vertices(G_T); ui != ui_end; ++ui)
	put(color, *ui, white(cc));

      typedef typename property_traits<FinishTime>::value_type D;
      typedef indirect_cmp< FinishTime, std::less<D> > Compare;

      Compare fl(f);
      priority_queue<Vertex, std::vector<Vertex>, Compare > Q(fl);

      typename graph_traits<Graph>::vertex_iterator i, j, iend, jend;
      tie(i, iend) = vertices(G_T);
      tie(j, jend) = vertices(G);
      for ( ; i != iend; ++i, ++j) {
	put(f, *i, get(f, *j));
	 Q.push(*i);
      }

      while ( !Q.empty() ) {
	Vertex u = Q.top();
	Q.pop();
	if  (get(color, u) == white(cc)) {
	  depth_first_visit(G_T, u, vis, color);
	  ++c_count; 
	}
      }
      return c_count;
    }
  } // namespace detail
  // Variant (3)
  template <class Graph, class Components,
            class DiscoverTime, class FinishTime,
            class Color, class DFSVisitor>
  inline typename property_traits<Components>::value_type
  connected_components(Graph& G,
                       Components c,
                       DiscoverTime d, 
                       FinishTime f,
                       Color color, DFSVisitor v) {

    typedef typename graph_traits<Graph>::directed_category Cat;
    return detail::connected_components(G, v, c, d, f, color, Cat());
  }
 
  // Variant (2)
  template <class Graph, class Components,
            class Color, class DFSVisitor>
  inline typename property_traits<Components>::value_type
  connected_components(Graph& G, Components c,
                       Color color, DFSVisitor v) {
    typedef typename graph_traits<Graph>::directed_category Cat;
    return detail::connected_components(G, v, c, color, Cat());
  }

  namespace detail {

    template <class Graph, class DFSVisitor, class Components>
    inline typename property_traits<Components>::value_type
    connected_components(Graph& G, DFSVisitor v, Components c, 
			 undirected_tag) 
    {
      return connected_components(G, v, c, 
		   get_vertex_property_accessor(G, color_tag()), 
		   undirected_tag());
    }

    template <class Graph, class DFSVisitor, class Components>
    inline typename property_traits<Components>::value_type
    connected_components(Graph& G, DFSVisitor v, Components c,
			  directed_tag)
    {
      return connected_components(G, v, c, 
		     get_vertex_property_accessor(G, discover_time_tag()),
		     get_vertex_property_accessor(G, finish_time_tag()),
		     get_vertex_property_accessor(G, color_tag()),
		     directed_tag());
    }

    template <class Graph, class DFSVisitor,
	      class Components,
	      class Color>
    inline typename property_traits<Components>::value_type
    connected_components(Graph& G, DFSVisitor v, Components c,
			 Color color, directed_tag)
    {
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      return connected_components(G, v, c, 
		   get_vertex_property_accessor(G, discover_time_tag()),
		   get_vertex_property_accessor(G, finish_time_tag()),
		   color, directed_tag());
    }
  } // namespace detail

  // Variant (1)
  template <class Graph, class Components, class DFSVisitor>
  inline typename property_traits<Components>::value_type
  connected_components(Graph& G, Components c, DFSVisitor v) {
    typedef typename graph_traits<Graph>::directed_category Cat;
    return detail::connected_components(G, v, c, Cat());
  }

  template <class Graph, class Components>
  inline typename property_traits<Components>::value_type
  connected_components(Graph& G, Components c) {
    return connected_components(G, c, dfs_visitor<>());
  }

  
  // A connected component algorithm for the case when dynamically
  // adding edges is common.  This is a preparing operation. Call
  // same_component to check whether two vertices are in the same
  // component, or use disjoint_set::find_set to determine
  // the representative for a vertex. 

  template <class VertexListGraph, class DisjointSets> 
  void
  initialize_dynamic_components(VertexListGraph& G,
                                DisjointSets& ds)
  {
    typename graph_traits<VertexListGraph>
      ::vertex_iterator v, vend;
    for (tie(v, vend) = vertices(G); v != vend; ++v)
      ds.make_set(*v);
  }

  template <class Vertex, class DisjointSet>
  inline bool
  same_component(Vertex u, Vertex v, DisjointSet& ds) {
    return ds.find_set(u) == ds.find_set(v);
  }

  /*****   
    Next is the implementation of the EdgeList version
    of connected components adapted from the implementation
    by Alex Stepanov
    *****/

} // namespace boost

#include <boost/graph/detail/connected_components.hpp>

namespace boost {

  // This version of connected components does not require a full
  // Graph. Instead, it just needs an edge list, where the vertices of
  // each edge need to be of integer type. The edges are assumed to
  // be undirected. The other difference is that the result is stored in
  // a container, instead of just a decorator.  The container should be
  // empty before the algorithm is called. It will grow during the
  // course of the algorithm. The container must be a model of
  // BackInsertionSequence and RandomAccessContainer
  // (std::vector is a good choice). After running the algorithm the
  // index container will map each vertex to the representative
  // vertex of the component to which it belongs.
  //
  // Adapted from an implementation by Alex Stepanov, with parts
  // inspired by Tarjan's "Data Structures and Network Algorithms"
  
  /*
    RankContainer is a random accessable container (operator[] is defined) 
    with a value type that can represent an integer part of a binary log of
    the value type of the corresponding ParentContainer (char is always enough)
    its size_type is no less than the size_type of the corresponding
    ParentContainer
  */
  
  template <class EdgeListGraph, class DisjointSets>
  void
  dynamic_connected_components(EdgeListGraph& g, DisjointSets& ds)
  {
    typename graph_traits<EdgeListGraph>::edge_iterator e, end;
    for (tie(e,end) = edges(g); e != end; ++e)
      ds.link(source(*e,g),target(*e,g));
  }
  
  template <class ParentIterator>
  void
  compress_components(ParentIterator first, ParentIterator last) {
    for (ParentIterator current = first; current != last; ++current) 
      detail::find_representative_with_full_compression(first, current-first);
  }
  
  template <class ParentIterator>
#if !defined BOOST_NO_STD_ITERATOR_TRAITS
  typename std::iterator_traits<ParentIterator>::difference_type
#else
  ptrdiff_t
#endif
  component_count(ParentIterator first, ParentIterator last)
  {
    ptrdiff_t count = 0;
    for (ParentIterator current = first; current != last; ++current) 
      if (*current == current - first) ++count; 
    return count;
  }
  
  // This algorithm can be applied to the result container of the
  // connected_components algorithm to normalize
  // the components.
  template <class ParentIterator>
  void normalize_components(ParentIterator first, ParentIterator last)
  {
    for (ParentIterator current = first; current != last; ++current) 
      detail::normalize_node(first, current - first);
  }
  
  // considering changing the so that it initializes with a pair of
  // vertex iterators and a parent PA.
  
  template <class IndexT>
  class component_index {
  public://protected: (avoid friends for now)
    typedef std::vector<IndexT> MyIndexContainer;
    MyIndexContainer header;
    MyIndexContainer index;
    typedef typename MyIndexContainer::size_type SizeT;
    typedef typename MyIndexContainer::const_iterator IndexIter;
  public:
    typedef detail::component_iterator<IndexIter, IndexT, SizeT> 
    component_iterator;
    class component {
      friend class component_index;
    protected:
      IndexT number;
      const component_index<IndexT>* comp_ind_ptr;
      component(IndexT i, const component_index<IndexT>* p) 
	: number(i), comp_ind_ptr(p) {}
    public:
      typedef component_iterator iterator;
      typedef component_iterator const_iterator;
      typedef IndexT value_type;
      iterator begin() const {
	return iterator( comp_ind_ptr->index.begin(),
			 (comp_ind_ptr->header)[number] );
      }
      iterator end() const {
	return iterator( comp_ind_ptr->index.begin(), 
			 comp_ind_ptr->index.size() );
      }
    };
    typedef SizeT size_type;
    typedef component value_type;
    
    template <class Iterator>
    component_index(Iterator first, Iterator last) 
#ifndef BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
      : index(first, last)
#else
    : index(last - first)
#endif
    { 
#ifdef BOOST_NO_TEMPLATED_ITERATOR_CONSTRUCTORS
      std::copy(first, last, index.begin());
#endif
      detail::construct_component_index(index, header);
  } 
    component operator[](IndexT i) const {
      return component(i, this);
    }
    SizeT size() {
      return header.size();
    }
    
  };
  
} /*namespace boost*/


#endif /*BOOST_GRAPH_CONNECTED_COMPONENTS_H*/
