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

#ifndef BOOST_GRAPH_GRAPH_SEARCH_VISITORS_HPP
#define BOOST_GRAPH_GRAPH_SEARCH_VISITORS_HPP

#include <iosfwd>
#include <boost/config.hpp>
#include <boost/property_accessor.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/pending/limits.hpp>
#include <boost/graph/detail/is_same.hpp>

namespace boost {

  // This is a bit more convenient than std::numeric_limits because
  // you don't have to explicitly provide type T.
  template <class T>
  inline T numeric_limits_max(T) { return std::numeric_limits<T>::max(); }

  //========================================================================
  // Event Tags

  // graph_search
  struct on_initialize_vertex { };
  struct on_start_vertex { };
  struct on_discover_vertex { };
  struct on_examine_edge { };
  struct on_tree_edge { };
  struct on_cycle_edge { };
  struct on_finish_vertex { };
  
  struct on_forward_or_cross_edge { };
  struct on_back_edge { };

  struct on_edge_relaxed { };
  struct on_edge_not_relaxed { };
  struct on_edge_minimized { };
  struct on_edge_not_minimized { };

  enum { FALSE_TAG, TRUE_TAG };

  struct true_tag { enum { num = TRUE_TAG }; };
  struct false_tag { enum { num = FALSE_TAG }; };

  //========================================================================
  // null_visitor and base_visitor

  struct null_visitor {
    typedef void event_filter;
    template <class T, class Graph>
    void operator()(T, Graph&) { }
  };
  // needed for MSVC workaround
  template <class Visitor>
  struct base_visitor {
    typedef void event_filter;
    template <class T, class Graph>
    void operator()(T, Graph&) { }
  };

  //========================================================================
  // The invoke_visitors() function

  namespace detail {
    template <class Visitor, class T, class Graph>
    inline void
    invoke_dispatch(Visitor& v, T x, Graph& g, true_tag) {
       v(x, g);
    }
    template <class Visitor, class T, class Graph>
    inline void
    invoke_dispatch(Visitor&, T, Graph&, false_tag) { }
  } // namespace detail

  template <class Visitor, class Rest, class T, class Graph, class Tag>
  inline void 
  invoke_visitors(std::pair<Visitor, Rest>& vlist, T x, Graph& g, Tag tag) {
    typedef typename Visitor::event_filter Category;
    typedef typename detail::is_same<Category, Tag>::is_same_tag IsSameTag;
    detail::invoke_dispatch(vlist.first, x, g, IsSameTag());
    invoke_visitors(vlist.second, x, g, tag);
  }
#if (defined BOOST_MSVC) 
  template <class Visitor, class T, class Graph, class Tag>
  inline void 
  invoke_visitors(base_visitor<Visitor>& v, T x, Graph& g, Tag tag) {
    typedef typename Visitor::event_filter Category;
    typedef typename detail::is_same<Category, Tag>::is_same_tag IsSameTag;
    Visitor& v = static_cast<Visitor&>(vis);
    detail::invoke_dispatch(v, x, g, IsSameTag());
  }
#else
  template <class Visitor, class T, class Graph, class Tag>
  inline void 
  invoke_visitors(Visitor& v, T x, Graph& g, Tag tag) {
    typedef typename Visitor::event_filter Category;
    typedef typename detail::is_same<Category, Tag>::is_same_tag IsSameTag;
    detail::invoke_dispatch(v, x, g, IsSameTag());
  }
#endif

  //========================================================================
  // predecessor_recorder

  template <class PredecessorPA, class Tag>
  struct predecessor_recorder
    : public base_visitor<predecessor_recorder<PredecessorPA, Tag> >
  {
    typedef Tag event_filter;
    predecessor_recorder(PredecessorPA pa) : m_predecessor(pa) { }
    template <class Edge, class Graph>
    void operator()(Edge e, const Graph& g) {
      put(m_predecessor, target(e, g), source(e, g));
    }
    PredecessorPA m_predecessor;
  };
  template <class PredecessorPA, class Tag>
  predecessor_recorder<PredecessorPA, Tag> 
  record_predecessors(PredecessorPA pa, Tag) {
    return predecessor_recorder<PredecessorPA, Tag> (pa);
  }

  //========================================================================
  // distance_recorder

  template <class DistancePA, class Tag>
  struct distance_recorder
    : public base_visitor<distance_recorder<DistancePA, Tag> >
  {
    typedef Tag event_filter;
    distance_recorder(DistancePA pa) : m_distance(pa) { }
    template <class Edge, class Graph>
    void operator()(Edge e, const Graph& g) {
      typename graph_traits<Graph>::vertex_descriptor 
	u = source(e, g), v = target(e, g);
      put(m_distance, v, get(m_distance, u) + 1);
    }
    DistancePA m_distance;
  };
  template <class DistancePA, class Tag>
  distance_recorder<DistancePA, Tag> 
  record_distances(DistancePA pa, Tag) {
    return distance_recorder<DistancePA, Tag> (pa);
  }

  //========================================================================
  // time_stamper

  
  template <class TimePA, class TimeT, class Tag>
  struct time_stamper
    : public base_visitor<time_stamper<TimePA, TimeT, Tag> >
  {
    typedef Tag event_filter;
    time_stamper(TimePA pa, TimeT& t) : m_time_pa(pa), m_time(t) { }
    template <class Vertex, class Graph>
    void operator()(Vertex u, const Graph& g) {
      put(m_time_pa, u, ++m_time);
    }
    TimePA m_time_pa;
    TimeT& m_time;
  };
  template <class TimePA, class TimeT, class Tag>
  time_stamper<TimePA, TimeT, Tag> 
  stamp_times(TimePA pa, TimeT& time_counter, Tag) {
    return time_stamper<TimePA, TimeT, Tag>(pa, time_counter);
  }

  //========================================================================
  // property_writer

  template <class PA, class OutputIterator, class Tag>
  struct property_writer
    : public base_visitor<property_writer<PA, OutputIterator, Tag> >
  {
    typedef Tag event_filter;

    property_writer(PA pa, OutputIterator out) : m_pa(pa), m_out(out) { }

    template <class T, class Graph>
    void operator()(T x, Graph& g) { *m_out++ = get(m_pa, x); }
    PA m_pa;
    OutputIterator m_out;
  };
  template <class PA, class OutputIterator, class Tag>
  property_writer<PA, OutputIterator, Tag>
  write_property(PA pa, OutputIterator out, Tag) {
    return property_writer<PA, OutputIterator, Tag>(pa, out);
  }

} /* namespace boost */

#endif
