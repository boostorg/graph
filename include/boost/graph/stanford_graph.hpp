//=======================================================================
// Copyright 1997-2001 University of Notre Dame.
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
#ifndef BOOST_GRAPH_SGB_GRAPH_HPP
#define BOOST_GRAPH_SGB_GRAPH_HPP

#include <boost/config.hpp>
#include <boost/iterator.hpp>
#include <boost/operators.hpp>
#include <boost/property_map.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

// Thanks to Andreas Scherer for numerous suggestions and fixes!

// This file adapts a Stanford GraphBase (SGB) Graph pointer into a
// VertexListGraph. Note that a graph adaptor class is not needed, 
// SGB's Graph* is used as is. The VertexListGraph concept is fulfilled by
// defining the appropriate non-member functions for Graph*.
//
// The PROTOTYPES change file extensions to SGB must be applied so
// that the SGB functions have real prototypes which are necessary for
// the C++ compiler. To apply the PROTOTYPES extensions, before you do
// "make tests install" for SGB do "ln -s PROTOTYPES/* ." to the SGB
// root directory (or just copy all the files from the PROTOTYPES
// directory to the SGB root directory).
//
extern "C" {
	// We include all global definitions for the general stuff
	// of The Stanford GraphBase and its various graph generator
	// functions by reading all SGB headerfiles as in section 2 of
	// the "test_sample" program.
#include <gb_graph.h> /* SGB data structures */
#include <gb_io.h> /* SGB input/output routines */
#include <gb_flip.h> /* random number generator */
#include <gb_dijk.h> /* routines for shortest paths */
#include <gb_basic.h> /* the basic graph operations */
#undef empty /* avoid name clash with C++ standard library */
	inline Graph* empty( long n ) /* and provide workaround */
	{ return board(n,0L,0L,0L,2L,0L,0L); }
#include <gb_books.h> /* graphs based on literature */
#include <gb_econ.h> /* graphs based on economic data */
#include <gb_games.h> /* graphs based on football scores */
#include <gb_gates.h> /* graphs based on logic circuits */
#undef val /* avoid name clash with g++ headerfile stl_tempbuf.h */
	// val ==> Vertex::x.I
#include <gb_lisa.h> /* graphs based on Mona Lisa */
#include <gb_miles.h> /* graphs based on mileage data */
#include <gb_plane.h> /* planar graphs */
#include <gb_raman.h> /* Ramanujan graphs */
#include <gb_rand.h> /* random graphs */
#include <gb_roget.h> /* graphs based on Roget's Thesaurus */
#include <gb_save.h> /* we save results in ASCII format */
#include <gb_words.h> /* five-letter-word graphs */
#undef weight /* avoid name clash with BGL parameter */
	// weight ==> Vertex::u.I
}

namespace boost {
  class sgb_edge;
}

class sgb_out_edge_iterator;
class sgb_adj_iterator;
class sgb_vertex_iterator;

namespace boost {
  template <> struct graph_traits<Graph*> {
    typedef Vertex* vertex_descriptor;
    typedef boost::sgb_edge edge_descriptor;
    typedef sgb_out_edge_iterator out_edge_iterator;
    typedef void in_edge_iterator;
    typedef sgb_adj_iterator adjacency_iterator;
    typedef sgb_vertex_iterator vertex_iterator;
    typedef void edge_iterator;
    typedef long vertices_size_type;
    typedef long edge_size_type;
    typedef long degree_size_type;
    typedef directed_tag directed_category;
    typedef vertex_list_graph_tag traversal_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
  };
  template <> struct graph_traits<Graph*const> {
    typedef Vertex* vertex_descriptor;
    typedef boost::sgb_edge edge_descriptor;
    typedef sgb_out_edge_iterator out_edge_iterator;
    typedef void in_edge_iterator;
    typedef sgb_adj_iterator adjacency_iterator;
    typedef sgb_vertex_iterator vertex_iterator;
    typedef void edge_iterator;
    typedef long vertices_size_type;
    typedef long edge_size_type;
    typedef long degree_size_type;
    typedef directed_tag directed_category;
    typedef vertex_list_graph_tag traversal_category;
    typedef allow_parallel_edge_tag edge_parallel_category;
  };
}

namespace boost {

  // We could just use Arc* as the edge descriptor type, but
  // we want to add the source(e,g) function which requires
  // that we carry along a pointer to the source vertex.
  class sgb_edge {
    typedef sgb_edge self;
  public:
    sgb_edge() : _arc(0), _src(0) { }
    sgb_edge(Arc* a, Vertex* s) : _arc(a), _src(s) { }
    friend Vertex* source(self& e, Graph*) { return e._src; }
    friend Vertex* target(self& e, Graph*) { return e._arc->tip; }
    friend bool operator==(const self& a, const self& b) {
      return a._arc == b._arc; }
    friend bool operator!=(const self& a, const self& b) {
      return a._arc != b._arc; }
#if !defined(BOOST_NO_MEMBER_TEMPLATE_FRIENDS)
    friend class sgb_edge_length_map;
    template <class Tag> friend class sgb_edge_util_map;
  protected:
#endif
    Arc* _arc;
    Vertex* _src;
  };
} // namespace boost

  class sgb_out_edge_iterator
    : public boost::forward_iterator_helper<
        sgb_out_edge_iterator, boost::sgb_edge, 
        std::ptrdiff_t, boost::sgb_edge*, boost::sgb_edge>
  {
    typedef sgb_out_edge_iterator self;
  public:
    sgb_out_edge_iterator() : _src(0), _arc(0) {}
    sgb_out_edge_iterator(Vertex* s, Arc* d) : _src(s), _arc(d) {}
    boost::sgb_edge operator*() { return boost::sgb_edge(_arc, _src); }
    self& operator++() { _arc = _arc->next; return *this; }
    friend bool operator==(const self& x, const self& y) {
      return x._arc == y._arc; }
  protected:
    Vertex* _src;
    Arc* _arc;
  };

  class sgb_adj_iterator
    : public boost::forward_iterator_helper<
        sgb_adj_iterator, Vertex*, std::ptrdiff_t, Vertex**,Vertex*>
  {
    typedef sgb_adj_iterator self;
  public:
    sgb_adj_iterator() : _arc(0) {}
    sgb_adj_iterator(Arc* d) : _arc(d) {}
    Vertex* operator*() { return _arc->tip; }
    self& operator++() { _arc = _arc->next; return *this; }
    friend bool operator==(const self& x, const self& y) {
      return x._arc == y._arc; }
  protected:
    Arc* _arc;
  };

  // The reason we have this instead of just using Vertex* is that we
  // want to use Vertex* as the vertex_descriptor instead of just
  // Vertex, which avoids problems with boost passing vertex descriptors
  // by value and how that interacts with the sgb_vertex_id_map.
  class sgb_vertex_iterator
    : public boost::forward_iterator_helper<
        sgb_vertex_iterator, Vertex*, std::ptrdiff_t, Vertex**, Vertex*>
  {
    typedef sgb_vertex_iterator self;
  public:
    sgb_vertex_iterator() : _v(0) { }
    sgb_vertex_iterator(Vertex* v) : _v(v) { }
    Vertex* operator*() { return _v; }
    self& operator++() { ++_v; return *this; }
    friend bool operator==(const self& x, const self& y) {
      return x._v == y._v; }
  protected:
    Vertex* _v;
  };

namespace boost {

  inline std::pair<sgb_vertex_iterator,sgb_vertex_iterator>
  vertices(Graph* g)
  {
    return std::make_pair(sgb_vertex_iterator(g->vertices),
			  sgb_vertex_iterator(g->vertices + g->n));
  }

  inline std::pair<sgb_out_edge_iterator,sgb_out_edge_iterator>
  out_edges(Vertex* u, Graph*)
  {
    return std::make_pair( sgb_out_edge_iterator(u, u->arcs),
                           sgb_out_edge_iterator(u, 0) );
  }

  inline boost::graph_traits<Graph*>::degree_size_type
  out_degree(Vertex* u, Graph* g)
  {
    boost::graph_traits<Graph*>::out_edge_iterator i, i_end;
    boost::tie(i, i_end) = out_edges(u, g);
    return std::distance(i, i_end);
  }

  // in_edges?

  inline std::pair<sgb_adj_iterator,sgb_adj_iterator>
  adjacent_vertices(Vertex* u, Graph*)
  {
    return std::make_pair( sgb_adj_iterator(u->arcs),
                           sgb_adj_iterator(0) );
  }

  inline long num_vertices(Graph* g) { return g->n; }
  inline long num_edges(Graph* g) { return g->m; }

  inline Vertex* vertex(long v, Graph* g) { return g->vertices + v; }

  // Various Property Maps

  // Vertex ID
  class sgb_vertex_id_map
    : public boost::put_get_at_helper<long, sgb_vertex_id_map>
  {
  public:
    typedef boost::readable_property_map_tag category;
    typedef long value_type;
    typedef Vertex* key_type;
    sgb_vertex_id_map() : _g(0) { }
    sgb_vertex_id_map(Graph* g) : _g(g) { }
    long operator[](Vertex* v) const { return v - _g->vertices; }
  protected:
    Graph* _g;
  };
  inline sgb_vertex_id_map get(vertex_index_t, Graph* g) {
    return sgb_vertex_id_map(g);
  }

  // Vertex Name  
  class sgb_vertex_name_t_map
    : public boost::put_get_at_helper<char*, sgb_vertex_name_t_map>
  {
  public:
    typedef boost::readable_property_map_tag category;
    typedef char* value_type;
    typedef Vertex* key_type;
    char* operator[](Vertex* v) const { return v->name; }
  };
  inline sgb_vertex_name_t_map get(vertex_name_t, Graph*) {
    return sgb_vertex_name_t_map();
  }

  // Vertex Property Tags
#define SGB_PROPERTY_TAG(KIND,TAG) \
  template <class T> struct TAG##_property { \
    typedef KIND##_property_tag kind; \
    typedef T type; \
  };
  SGB_PROPERTY_TAG(vertex, u)
  SGB_PROPERTY_TAG(vertex, v)
  SGB_PROPERTY_TAG(vertex, w)
  SGB_PROPERTY_TAG(vertex, x)
  SGB_PROPERTY_TAG(vertex, y)
  SGB_PROPERTY_TAG(vertex, z)
 
  // Edge Property Tags
  SGB_PROPERTY_TAG(edge, a)
  SGB_PROPERTY_TAG(edge, b)

  struct edge_length_t {
    typedef edge_property_tag kind;
  };

  // Various Utility Maps

  // helpers
  inline Vertex*& get_util(util& u, Vertex*) { return u.V; }
  inline Arc*& get_util(util& u, Arc*) { return u.A; }
  inline Graph*& get_util(util& u, Graph*) { return u.G; }
  inline char*& get_util(util& u, char*) { return u.S; }
  inline long& get_util(util& u, long) { return u.I; }

#define SGB_GET_UTIL_FIELD(KIND,X) \
  template <class T> \
  inline T& get_util_field(KIND* k, X##_property<T>) { \
    return get_util(k->X, T());  }

  SGB_GET_UTIL_FIELD(Vertex, u)
  SGB_GET_UTIL_FIELD(Vertex, v)
  SGB_GET_UTIL_FIELD(Vertex, w)
  SGB_GET_UTIL_FIELD(Vertex, x)
  SGB_GET_UTIL_FIELD(Vertex, y)
  SGB_GET_UTIL_FIELD(Vertex, z)

  SGB_GET_UTIL_FIELD(Arc, a)
  SGB_GET_UTIL_FIELD(Arc, b)

  // Vertex Utility Map
  template <class Tag>
  class sgb_vertex_util_map
    : public boost::put_get_at_helper< typename Tag::type,
        sgb_vertex_util_map<Tag> >
  {
  public:
    typedef boost::lvalue_property_map_tag category;
    typedef typename Tag::type value_type;
    typedef Vertex* key_type;
    const value_type& operator[](Vertex* v) const {
      return get_util_field(v, Tag()); 
    }
    value_type& operator[](Vertex* v) {
      return get_util_field(v, Tag()); 
    }
  };

  // Edge Utility Map
  template <class Tag>
  class sgb_edge_util_map
    : public boost::put_get_at_helper< typename Tag::type,
        sgb_edge_util_map<Tag> >
  {
  public:
    typedef boost::lvalue_property_map_tag category;
    typedef typename Tag::type value_type;
    typedef Vertex* key_type;
    value_type& operator[](const sgb_edge& e) {
      return get_util_field(e._arc, Tag()); 
    }
    const value_type& operator[](const sgb_edge& e) const {
      return get_util_field(e._arc, Tag()); 
    }
  };

#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION  

  template <class Tag>
  inline sgb_vertex_util_map<Tag>
  get_property_map(Tag, const Graph* g, vertex_property_tag) {
    return sgb_vertex_util_map<Tag>();
  }
  template <class Tag>
  inline sgb_edge_util_map<Tag> 
  get_property_map(Tag, const Graph* g, edge_property_tag) {
    return sgb_edge_util_map<Tag>();
  }

  template <class Tag, class Kind>  
  struct sgb_util_map { };

  template <class Tag> struct sgb_util_map<Tag, vertex_property_tag> {
    typedef typename sgb_vertex_util_map<Tag>::type type;
  };
  template <class Tag> struct sgb_util_map<Tag, edge_property_tag> {
    typedef typename sgb_edge_util_map<Tag>::type type;
  };

#endif // ! BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

  // Edge Length Access
  class sgb_edge_length_map
    : public boost::put_get_at_helper<long, sgb_edge_length_map>
  {
  public:
    typedef boost::lvalue_property_map_tag category;
    typedef long value_type;
    typedef sgb_edge key_type;
    long& operator[](const sgb_edge& e) { 
      return e._arc->len; 
    }
    const long& operator[](const sgb_edge& e) const {
      return e._arc->len; 
    }
  };
  inline sgb_edge_length_map
  get(edge_length_t, const Graph*) { 
    return sgb_edge_length_map(); 
  }
  inline sgb_edge_length_map::value_type
  get(edge_length_t, const Graph*, const sgb_edge_length_map::key_type& key) {
    return sgb_edge_length_map()[key];
  }
  inline void
  put(edge_length_t, Graph*, const sgb_edge_length_map::key_type& key,
      const sgb_edge_length_map::value_type& value)
  {
    sgb_edge_length_map()[key] = value;
  }

  // Property Map Traits Classes
  template <>
  struct property_map<Graph*, edge_length_t> {
    typedef sgb_edge_length_map type;
    typedef sgb_edge_length_map const_type;
  };
  template <>
  struct property_map<Graph*, vertex_index_t> {
    typedef sgb_vertex_id_map type;
    typedef sgb_vertex_id_map const_type;
  };
  template <>
  struct property_map<Graph*, vertex_name_t> {
    typedef sgb_vertex_name_t_map type;
    typedef sgb_vertex_name_t_map const_type;
  };

#if !defined(BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION)

  namespace detail {
    template <class Kind, class PropertyTag>
    struct sgb_choose_property_map { };
    template <class PropertyTag>
    struct sgb_choose_property_map<vertex_property_tag, PropertyTag> {
      typedef sgb_vertex_util_map<PropertyTag> type;
    };
    template <class PropertyTag>
    struct sgb_choose_property_map<edge_property_tag, PropertyTag> {
      typedef sgb_edge_util_map<PropertyTag> type;
    };
  } // namespace detail
  template <class PropertyTag>
  struct property_map<Graph*, PropertyTag> {
    typedef typename property_kind<PropertyTag>::type Kind;
    typedef typename detail::sgb_choose_property_map<Kind, 
      PropertyTag>::type type;
    typedef type const_type;
  };

#define SGB_UTIL_ACCESSOR(KIND,X) \
  template <class T> \
  inline sgb_##KIND##_util_map< X##_property<T> > \
  get(X##_property<T>, Graph*) { \
    return sgb_##KIND##_util_map< X##_property<T> >(); \
  } \
  template <class T, class Key> \
  inline typename sgb_##KIND##_util_map< X##_property<T> >::value_type \
  get(X##_property<T>, const Graph*, const Key& key) { \
    return sgb_##KIND##_util_map< X##_property<T> >()[key]; \
  } \
  template <class T, class Key, class Value> \
  inline  void \
  put(X##_property<T>, Graph*, const Key& key, const Value& value) { \
    sgb_##KIND##_util_map< X##_property<T> >()[key] = value; \
  }

#else // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

#define SGB_UTIL_ACCESSOR_TYPE(KIND,TAG,TYPE) \
  inline sgb_##KIND##_util_map< TAG<TYPE> > \
  get(TAG<TYPE>, Graph*) { \
    return sgb_##KIND##_util_map< TAG<TYPE> >(); \
  } \
  template <class Key> \
  inline typename sgb_##KIND##_util_map< TAG<TYPE> >::value_type \
  get(TAG<TYPE>, const Graph*, const Key& key) { \
    return sgb_##KIND##_util_map< TAG<TYPE> >()[key]; \
  } \
  template <class Key, class Value> \
  inline  void \
  put(TAG<TYPE>, Graph*, const Key& key, const Value& value) { \
    sgb_##KIND##_util_map< TAG<TYPE> >()[key] = value; \
  } \
  template <> struct property_map<Graph*, TAG<TYPE> > { \
    typedef sgb_##KIND##_util_map< TAG<TYPE> > type; \
  }

#define SGB_UTIL_ACCESSOR(KIND,TAG) \
  SGB_UTIL_ACCESSOR_TYPE(KIND, TAG##_property, Vertex*); \
  SGB_UTIL_ACCESSOR_TYPE(KIND, TAG##_property, Arc*); \
  SGB_UTIL_ACCESSOR_TYPE(KIND, TAG##_property, Graph*); \
  SGB_UTIL_ACCESSOR_TYPE(KIND, TAG##_property, long); \
  SGB_UTIL_ACCESSOR_TYPE(KIND, TAG##_property, char*);

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

  SGB_UTIL_ACCESSOR(vertex, u)
  SGB_UTIL_ACCESSOR(vertex, v)
  SGB_UTIL_ACCESSOR(vertex, w)
  SGB_UTIL_ACCESSOR(vertex, x)
  SGB_UTIL_ACCESSOR(vertex, y)
  SGB_UTIL_ACCESSOR(vertex, z)

  SGB_UTIL_ACCESSOR(edge, a)
  SGB_UTIL_ACCESSOR(edge, b)

} // namespace boost

#endif // BOOST_GRAPH_SGB_GRAPH_HPP
