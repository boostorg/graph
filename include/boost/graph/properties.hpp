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
#ifndef BOOST_GRAPH_PROPERTIES_HPP
#define BOOST_GRAPH_PROPERTIES_HPP

#include <boost/config.hpp>
#include <boost/property_accessor.hpp>

namespace boost {

  enum default_color_type { white_color, gray_color, black_color };

  inline default_color_type white(default_color_type) { return white_color; }
  inline default_color_type gray(default_color_type) { return gray_color; }
  inline default_color_type black(default_color_type) { return black_color; }

  namespace detail {
    // These enums are only used in the no partial specialzation workaround
    enum property_tag_num
    {
      NO_PLUGIN_TAG, ID_PLUGIN_TAG, NAME_PLUGIN_TAG, WEIGHT_PLUGIN_TAG, 
      DISTANCE_PLUGIN_TAG, COLOR_PLUGIN_TAG, DEGREE_PLUGIN_TAG,
      OUT_DEGREE_PLUGIN_TAG, IN_DEGREE_PLUGIN_TAG, DISCOVER_TIME_PLUGIN_TAG,
      FINISH_TIME_PLUGIN_TAG
    };
  } // namespace detail

  // The enum's are only necessary for a workaround for compilers that
  // don't do partial specialization (like VC++).

  struct id_tag {
    enum { num = detail::ID_PLUGIN_TAG };
  };
  struct name_tag { 
    enum { num = detail::NAME_PLUGIN_TAG };
  };
  struct weight_tag { 
    enum { num = detail::WEIGHT_PLUGIN_TAG };
  };
  struct distance_tag { 
    enum { num = detail::DISTANCE_PLUGIN_TAG };
  };
  struct color_tag { 
    enum { num = detail::COLOR_PLUGIN_TAG };
  };
  struct degree_tag { 
    enum { num = detail::DEGREE_PLUGIN_TAG };
  };
  struct out_degree_tag { 
    enum { num = detail::OUT_DEGREE_PLUGIN_TAG };
  };
  struct in_degree_tag { 
    enum { num = detail::IN_DEGREE_PLUGIN_TAG };
  };
  struct discover_time_tag { 
    enum { num = detail::DISCOVER_TIME_PLUGIN_TAG };
  };
  struct finish_time_tag { 
    enum { num = detail::FINISH_TIME_PLUGIN_TAG };
  };

  struct foo_edge_property_selector {
    template <class Graph, class Plugin, class Tag>
    struct bind {
      typedef void type;
      typedef void const_type;
    };
  };
  template <class GraphTag>
  struct edge_property_selector {
    typedef foo_edge_property_selector type;
  };

  struct foo_vertex_property_selector {
    template <class Graph, class Plugin, class Tag>
    struct bind {
      typedef void type;
      typedef void const_type;
    };
  };

  template <class GraphTag>
  struct vertex_property_selector {
    typedef foo_vertex_property_selector type;
  };

  template <class Graph, class PropertyTag>
  struct edge_property_accessor {
    typedef typename Graph::directed_category Directed;
    typedef typename Graph::edge_plugin_type Plugin;
    typedef typename Graph::graph_tag graph_tag;
    typedef typename edge_property_selector<graph_tag>::type Selector;
    typedef typename Selector::BOOST_TEMPLATE bind<Graph,Plugin,PropertyTag>
      Bind;
    typedef typename Bind::type type;
    typedef typename Bind::const_type const_type;
  };
  template <class Graph, class PropertyTag>
  class vertex_property_accessor {
    typedef typename Graph::vertex_plugin_type Plugin;
    typedef typename Graph::graph_tag graph_tag;
    typedef typename vertex_property_selector<graph_tag>::type Selector;
    typedef typename Selector::BOOST_TEMPLATE bind<Graph,Plugin,PropertyTag>
      Bind;
  public:
    typedef typename Bind::type type;
    typedef typename Bind::const_type const_type;
  };

#if 0 // UNDER CONSTRUCTION

  struct white_type { };
  struct gray_type { };
  struct black_type { };

  template <class T> white_type white(proxy) { return white_type(); }
  template <class T> gray_type gray(proxy) { return gray_type(); }
  template <class T> black_type black(proxy) { return black_type(); }

  template <class ColorAccessor>
  class color_reseting_adaptor
  {
    typedef typename property_traits<ColorAccessor>::value_type value_type;
    typedef typename property_traits<ColorAccessor>::key_type key_type;

    class proxy
    {
      proxy(default_color_type& c, const default_color_type& index)
	: m_color(c), m_color_index(index) { }

      proxy& operator=(const white_type&) {
	m_color = m_color_index - 1;
	return *this;
      }
      proxy& operator=(const gray_type&) {
	m_color = m_color_index;
	return *this;
      }
      proxy& operator=(const black_type&) {
	m_color = m_color_index + 1;
	return *this;
      }
      default_color_type& m_color;
      default_color_type m_color_index;
    };

    proxy operator[](const key_type& key) const {
      return proxy(get(m_color, key), m_color_index);
    }

    void reset() { ++m_color_index; }
    void set_index(const ColorValue& cv) { m_color_index = cv; }

    ColorAccessor m_color;
    ColorValue m_color_index;
  };

  inline bool operator==(proxy p, white_type)
  {
    return p.m_color < p.m_color_index;
  }
  inline bool operator==(white_type, proxy p)
  {
    return p.m_color < p.m_color_index;
  }

  inline bool operator==(proxy p, gray_type)
  {
    return p.m_color == p.m_color_index;
  }
  inline bool operator==(proxy p, black_type)
  {
    return p.m_color > p.m_color_index;
  }

  template <class ColorAccessor>
  struct ResetableColorAccessor_concept
  {
    void constraints() {
      REQUIRES(ColorAccessor, ReadWritePropertyAccessor);
      color.reset();
    }
    ColorAccessor color;
  };

#endif

} // namespace boost

#endif /* BOOST_GRAPH_PROPERTIES_HPPA */
