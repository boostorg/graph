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
#ifndef BOOST_GRAPH_SGB_GRAPH_HPP
#define BOOST_GRAPH_SGB_GRAPH_HPP

#include <utility>
#include <boost/config.hpp>
#include <boost/iterator.hpp>
#include <boost/operators.hpp>
#include <boost/property_accessor.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/properties.hpp>

// This file adapts a Stanford GraphBase Graph pointer into a
// VertexListGraph. Note that a graph adaptor class is not needed, the
// Graph* is used as is. The VertexListGraph concept is fulfilled by
// defining the appropriate non-member functions for Graph*.

extern "C" {
  // We do not include "gb_graph.h" because it contains C-style
  // no-parameter extern defs for functions like gb_recycle :( Here we
  // duplicate some of gb_graph.h.
  typedef union{
    struct vertex_struct*V;
    struct arc_struct*A;
    struct graph_struct*G;
    char*S;
    long I;
  } util;
  
  typedef struct vertex_struct{
    struct arc_struct*arcs;
    char*name;
    util u,v,w,x,y,z;
  } Vertex;

  typedef struct arc_struct{
    struct vertex_struct*tip;
    struct arc_struct*next;
    long len;
    util a,b;
  } Arc;

  struct area_pointers {
    char*first;
    struct area_pointers*next;
  };

  typedef struct area_pointers*Area[1];

  typedef struct graph_struct {
    Vertex*vertices;
    long n;
    long m;
    char id[161]; // warning, this was ID_FIELD_SIZE
    char util_types[15];
    Area data;
    Area aux_data;
    util uu,vv,ww,xx,yy,zz;
  } Graph;

  // Some functions and globals in libgb
  void gb_recycle(Graph*);
  extern long verbose;
  extern long panic_code;
  Graph* miles(unsigned long,long,long,long,unsigned long,unsigned long,long);
  Graph* restore_graph(char*);
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
    typedef allow_parallel_edge_tag edge_parallel_category;
  };
}

namespace boost {

  // We could just use Arc* as the edge descriptor type, but
  // we want to add the source(e,g) function which requires
  // that we carry along a pointer to the source vertex.
  class sgb_edge {
  public:
    inline sgb_edge() : _arc(0), _src(0) { }
    inline sgb_edge(Arc* a, Vertex* s) : _arc(a), _src(s) { }
    friend inline Vertex* source(sgb_edge e, Graph*) { return e._src; }
    friend inline Vertex* target(sgb_edge e, Graph*) { return e._arc->tip; }
    //  protected:
    Arc* _arc;
    Vertex* _src;
  };
  inline bool operator==(const sgb_edge& a, const sgb_edge& b) {
    return a._arc == b._arc;
  }
  inline bool operator!=(const sgb_edge& a, const sgb_edge& b) {
    return a._arc != b._arc;
  }

} // namespace boost

  struct sgb_out_edge_iterator
    : public boost::forward_iterator_helper<
        sgb_out_edge_iterator, boost::sgb_edge, 
        ptrdiff_t, boost::sgb_edge*, boost::sgb_edge>
  {
    typedef sgb_out_edge_iterator self;
    inline sgb_out_edge_iterator() : _src(0), _arc(0) {}
    inline sgb_out_edge_iterator(Vertex* s, Arc* d) : _src(s), _arc(d) {}
    inline boost::sgb_edge operator*() { return boost::sgb_edge(_arc, _src); }
    inline self& operator++() { _arc = _arc->next; return *this; }
    friend inline bool operator==(const self& x, const self& y) {
      return x._arc == y._arc; }
  protected:
    Vertex* _src;
    Arc* _arc;
  };

  struct sgb_adj_iterator
    : public boost::forward_iterator_helper<
        sgb_adj_iterator, Vertex*, ptrdiff_t, Vertex**,Vertex*>
  {
    typedef sgb_adj_iterator self;
    inline sgb_adj_iterator() : _arc(0) {}
    inline sgb_adj_iterator(Arc* d) : _arc(d) {}
    inline Vertex* operator*() { return _arc->tip; }
    inline self& operator++() { _arc = _arc->next; return *this; }
    friend inline bool operator==(const self& x, const self& y) {
      return x._arc == y._arc; }
  protected:
    Arc* _arc;
  };

  // The reason we have this instead of just using Vertex* is that we
  // want to use Vertex* as the vertex_descriptor instead of just
  // Vertex, which avoids problems with boost passing vertex descriptors
  // by value and how that interacts with the sgb_vertex_id_accessor.
  struct sgb_vertex_iterator
    : public boost::forward_iterator_helper<
        sgb_vertex_iterator, Vertex*, ptrdiff_t, Vertex**, Vertex*>
  {
    typedef sgb_vertex_iterator self;
    inline sgb_vertex_iterator() : _v(0) { }
    inline sgb_vertex_iterator(Vertex* v) : _v(v) { }
    inline Vertex* operator*() { return _v; }
    inline self& operator++() { ++_v; return *this; }
    friend inline bool operator==(const self& x, const self& y) {
      return x._v == y._v; }
  protected:
    Vertex* _v;
  };


#ifndef __GNUC__
namespace boost {
#endif

  inline std::pair<Vertex*,Vertex*>
  vertices(Graph* g)
  {
    return std::make_pair( g->vertices, g->vertices + g->n );
  }

  inline std::pair<sgb_out_edge_iterator,sgb_out_edge_iterator>
  out_edges(Vertex* u, Graph* g)
  {
    return std::make_pair( sgb_out_edge_iterator(u, u->arcs),
			   sgb_out_edge_iterator(u, 0) );
  }

  // in_edges?

  inline std::pair<sgb_adj_iterator,sgb_adj_iterator>
  adjacent_vertices(Vertex* u, Graph* g)
  {
    return std::make_pair( sgb_adj_iterator(u->arcs),
			   sgb_adj_iterator(0) );
  }
  // deprecated
  inline std::pair<sgb_adj_iterator,sgb_adj_iterator>
  adj(Vertex* u, Graph* g)
  {
    return adjacent_vertices(u, g);
  }

  inline long num_vertices(Graph* g) { return g->n; }
  inline long num_edges(Graph* g) { return g->m; }

  inline Vertex* vertex(long v, Graph* g) { return g->vertices + v; }

#ifdef __GNUC__
namespace boost {
#endif

  // Various Property Accessors

  // Vertex ID
  struct sgb_vertex_id_accessor
    : public boost::detail::put_get_at_helper<long, sgb_vertex_id_accessor>
  {
    typedef boost::readable_op_bracket_tag category;
    typedef long value_type;
    typedef Vertex* key_type;
    inline sgb_vertex_id_accessor() : _g(0) { }
    inline sgb_vertex_id_accessor(Graph* g) : _g(g) { }
    inline long operator[](Vertex* v) const { return v - _g->vertices; }
    Graph* _g;
  };
  inline sgb_vertex_id_accessor get_vertex_property_accessor(Graph* g, id_tag) {
    return sgb_vertex_id_accessor(g);
  }

  // Vertex Name  
  struct sgb_vertex_name_accessor
    : public boost::detail::put_get_at_helper<char*, sgb_vertex_name_accessor>
  {
    typedef boost::readable_op_bracket_tag category;
    typedef char* value_type;
    typedef Vertex* key_type;
    inline char* operator[](Vertex* v) const { return v->name; }
  };
  inline sgb_vertex_name_accessor get_vertex_property_accessor(Graph* g, name_tag) {
    return sgb_vertex_name_accessor();
  }

  // Vertex Property Tags
  template <class T> struct u_tag  { typedef T type; };
  template <class T> struct v_tag  { typedef T type; };
  template <class T> struct w_tag  { typedef T type; };
  template <class T> struct x_tag  { typedef T type; };
  template <class T> struct y_tag  { typedef T type; };
  template <class T> struct z_tag  { typedef T type; };
  // Edge Property Tags
  template <class T> struct a_tag  { typedef T type; };
  template <class T> struct b_tag  { typedef T type; };
  struct length_tag { };

  // Vertex Utility Accessor

  // helpers
  inline Vertex*& get_util(util& u, Vertex*) { return u.V; }
  inline Arc*& get_util(util& u, Arc*) { return u.A; }
  inline Graph*& get_util(util& u, Graph*) { return u.G; }
  inline char*& get_util(util& u, char*) { return u.S; }
  inline long& get_util(util& u, long) { return u.I; }

#define GET_VERTEX_UTIL_FIELD(X) \
  template <class T> \
  inline T& get_util_field(Vertex* v, X##_tag<T>) { \
    return get_util(v->X, T());  }

  GET_VERTEX_UTIL_FIELD(u)
  GET_VERTEX_UTIL_FIELD(v)
  GET_VERTEX_UTIL_FIELD(x)
  GET_VERTEX_UTIL_FIELD(y)
  GET_VERTEX_UTIL_FIELD(z)

#define GET_EDGE_UTIL_FIELD(X) \
  template <class T> \
  inline T& get_util_field(Arc* e, X##_tag<T>) { \
    return get_util(e->X, T());  }

  GET_EDGE_UTIL_FIELD(a)
  GET_EDGE_UTIL_FIELD(b)

  template <class Tag>
  struct sgb_vertex_util_accessor
    : public boost::detail::put_get_at_helper< typename Tag::type,
        sgb_vertex_util_accessor<Tag> >
  {
    typedef boost::lvalue_op_bracket_tag category;
    typedef typename Tag::type value_type;
    typedef Vertex* key_type;
    inline const value_type& operator[](Vertex* v) const {
      return get_util_field(v, Tag()); 
    }
    inline value_type& operator[](Vertex* v) {
      return get_util_field(v, Tag()); 
    }
  };
#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION
  template <class Tag>
  inline sgb_vertex_util_accessor<Tag>
  get_vertex_property_accessor(Graph* g, Tag) {
    return sgb_vertex_util_accessor<Tag>();
  }
#endif

  // Edge Length Access
  struct sgb_edge_length_accessor
    : public boost::detail::put_get_at_helper<long, sgb_edge_length_accessor>
  {
    typedef boost::lvalue_op_bracket_tag category;
    typedef long value_type;
    typedef sgb_edge key_type;
    inline long& operator[](const sgb_edge& e) { 
      return e._arc->len; 
    }
    inline const long& operator[](const sgb_edge& e) const {
      return e._arc->len; 
    }
  };
  inline sgb_edge_length_accessor
  get_edge_property_accessor(Graph* g, length_tag) { 
    return sgb_edge_length_accessor(); 
  }
  
  template <class Tag>
  struct sgb_edge_util_accessor
    : public boost::detail::put_get_at_helper< typename Tag::type,
        sgb_edge_util_accessor<Tag> >
  {
    typedef boost::lvalue_op_bracket_tag category;
    typedef typename Tag::type value_type;
    typedef Vertex* key_type;
    inline const value_type& operator[](sgb_edge e) const {
      return get_util_field(e._arc, Tag()); 
    }
    inline value_type& operator[](sgb_edge e) {
      return get_util_field(e._arc, Tag()); 
    }
  };
#if !defined BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION  
  template <class Tag>
  inline sgb_edge_util_accessor<Tag> 
  get_edge_property_accessor(Graph* g, Tag) {
    return sgb_edge_util_accessor<Tag>();
  }
#endif

  // Property Accessor Traits Classes

  template <>
  struct edge_property_accessor<Graph*,length_tag> {
    typedef sgb_edge_length_accessor type;
  };
  template <>
  struct vertex_property_accessor<Graph*,id_tag> {
    typedef sgb_vertex_id_accessor type;
  };
  template <>
  struct vertex_property_accessor<Graph*,name_tag> {
    typedef sgb_vertex_name_accessor type;
  };
#ifdef BOOST_GRAPH_PARTIAL_SPECIALIZATION  
  template <class PropertyTag>
  struct edge_property_accessor<Graph*,PropertyTag> {
    typedef sgb_edge_util_accessor<PropertyTag> type;
  };
  template <class PropertyTag>
  struct vertex_property_accessor<Graph*,PropertyTag> {
    typedef sgb_vertex_util_accessor<PropertyTag> type;
  };
#else

#define SGB_VERTEX_UTIL_ACCESSOR(TAG,TYPE) \
  inline sgb_vertex_util_accessor< TAG<TYPE> > \
  get_vertex_property_accessor(Graph* g, TAG<TYPE> ) { \
    return sgb_vertex_util_accessor< TAG<TYPE> >(); \
  } \
  template <> struct vertex_property_accessor<Graph*, TAG<TYPE> > { \
    typedef sgb_vertex_util_accessor< TAG<TYPE> > type; \
  }

SGB_VERTEX_UTIL_ACCESSOR(u_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(u_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(u_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(u_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(u_tag,char*);

SGB_VERTEX_UTIL_ACCESSOR(v_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(v_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(v_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(v_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(v_tag,char*);

SGB_VERTEX_UTIL_ACCESSOR(w_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(w_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(w_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(w_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(w_tag,char*);

SGB_VERTEX_UTIL_ACCESSOR(x_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(x_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(x_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(x_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(x_tag,char*);
  
SGB_VERTEX_UTIL_ACCESSOR(y_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(y_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(y_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(y_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(y_tag,char*);

SGB_VERTEX_UTIL_ACCESSOR(z_tag,Vertex*);
SGB_VERTEX_UTIL_ACCESSOR(z_tag,Arc*);
SGB_VERTEX_UTIL_ACCESSOR(z_tag,Graph*);
SGB_VERTEX_UTIL_ACCESSOR(z_tag,long);
SGB_VERTEX_UTIL_ACCESSOR(z_tag,char*);

#define SGB_EDGE_UTIL_ACCESSOR(TAG,TYPE) \
  inline sgb_edge_util_accessor< TAG<TYPE> > \
  get_edge_property_accessor(Graph* g, TAG<TYPE> ) { \
    return sgb_edge_util_accessor< TAG<TYPE> >(); \
  } \
  template <> struct edge_property_accessor<Graph*, TAG<TYPE> > { \
    typedef sgb_edge_util_accessor< TAG<TYPE> > type; \
  }

SGB_EDGE_UTIL_ACCESSOR(a_tag,Vertex*);
SGB_EDGE_UTIL_ACCESSOR(a_tag,Arc*);
SGB_EDGE_UTIL_ACCESSOR(a_tag,Graph*);
SGB_EDGE_UTIL_ACCESSOR(a_tag,long);
SGB_EDGE_UTIL_ACCESSOR(a_tag,char*);

SGB_EDGE_UTIL_ACCESSOR(b_tag,Vertex*);
SGB_EDGE_UTIL_ACCESSOR(b_tag,Arc*);
SGB_EDGE_UTIL_ACCESSOR(b_tag,Graph*);
SGB_EDGE_UTIL_ACCESSOR(b_tag,long);
SGB_EDGE_UTIL_ACCESSOR(b_tag,char*);

#endif // BOOST_NO_TEMPLATE_PARTIAL_SPECIALIZATION

} // namespace boost

#endif // BOOST_GRAPH_SGB_GRAPH_HPP
