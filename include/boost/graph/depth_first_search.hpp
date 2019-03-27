//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Copyright 2003 Bruce Barr
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// Nonrecursive implementation of depth_first_visit_impl submitted by
// Bruce Barr, schmoost <at> yahoo.com, May/June 2003.
#ifndef BOOST_GRAPH_RECURSIVE_DFS_HPP
#define BOOST_GRAPH_RECURSIVE_DFS_HPP

#include <boost/graph/named_function_params.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/config.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/mpl/vector.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/type_traits/add_pointer.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/declval.hpp>

#if defined(BOOST_NO_CXX11_DECLTYPE)
#include <boost/typeof/typeof.hpp>
#endif

#if !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
namespace boost { namespace detail {

    template <typename T, typename G>
    class has_finish_edge_impl
    {
        typedef typename boost::remove_const<T>::type _m_T;

        template <typename B, typename P>
        static graph_yes_tag
            _check(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().finish_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().finish_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check(...);

    public:
        typedef mpl::bool_<
            sizeof(
                has_finish_edge_impl<T,G>::_check(
                    static_cast<mpl::vector<T,G>*>(BOOST_GRAPH_DETAIL_NULLPTR)
                )
            ) == sizeof(graph_yes_tag)
        > type;
    };
}}
#endif  // !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)

namespace boost { namespace detail {

#if !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
    template <typename T, typename G>
    class is_dfs_visitor_impl
    {
        typedef typename boost::remove_const<T>::type _m_T;

        template <typename B, typename P>
        static graph_yes_tag
            _check_init_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().initialize_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().initialize_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_init_v(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_start_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().start_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().start_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_start_v(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_disc_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().discover_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().discover_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_disc_v(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_exam_e(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().examine_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().examine_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_exam_e(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_tree_e(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().tree_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().tree_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_tree_e(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_back_e(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().back_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().back_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_back_e(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_forc_e(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().forward_or_cross_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().forward_or_cross_edge(
                            boost::declval<
                                typename graph_traits<P>::edge_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_forc_e(...);

        template <typename B, typename P>
        static graph_yes_tag
            _check_end_v(
                mpl::vector<B,P>*,
                typename boost::add_pointer<
#if defined(BOOST_NO_CXX11_DECLTYPE)
                    BOOST_TYPEOF_KEYWORD((
                        boost::detail::declref<B>().finish_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    ))
#else
                    decltype(
                        boost::detail::declref<B>().finish_vertex(
                            boost::declval<
                                typename graph_traits<P>::vertex_descriptor
                            >(),
                            boost::detail::declcref<P>()
                        )
                    )
#endif
                >::type = BOOST_GRAPH_DETAIL_NULLPTR
            );

        static graph_no_tag _check_end_v(...);

    public:
        typedef mpl::bool_<
            (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_init_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_start_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_disc_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_exam_e(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_tree_e(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_back_e(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_forc_e(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            ) && (
                sizeof(
                    is_dfs_visitor_impl<T,G>::_check_end_v(
                        static_cast<mpl::vector<_m_T,G>*>(
                            BOOST_GRAPH_DETAIL_NULLPTR
                        )
                    )
                ) == sizeof(graph_yes_tag)
            )
        > type;
    };
}}

#include <boost/mpl/eval_if.hpp>

namespace boost { namespace detail {

    template <typename T, typename G>
    struct is_dfs_visitor
      : mpl::eval_if<
        is_bgl_graph<G>,
        is_dfs_visitor_impl<T,G>,
        mpl::false_
      >::type
    { };

    template <typename T, typename G>
    struct is_dfs_visitor_with_finish_edge
      : mpl::eval_if<
        is_dfs_visitor<T,G>,
        has_finish_edge_impl<T,G>,
        mpl::false_
      >::type
    { };

    typedef argument_with_graph_predicate<
      is_dfs_visitor
    > dfs_visitor_predicate;
#else   // defined(BOOST_NO_CXX11_DECLTYPE) && !defined(BOOST_TYPEOF_KEYWORD)
    typedef visitor_predicate dfs_visitor_predicate;
#endif  // !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD)
}}
#endif  // defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)

#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/detail/mpi_include.hpp>
#include <boost/parameter.hpp>
#include <boost/ref.hpp>
#include <boost/implicit_cast.hpp>
#include <boost/optional.hpp>
#include <boost/functional/value_factory.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/concept/assert.hpp>
#include <vector>
#include <utility>

#if !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/core/enable_if.hpp>
#endif

namespace boost {

  template <class Visitor, class Graph>
  class DFSVisitorConcept {
  public:
    void constraints() {
      BOOST_CONCEPT_ASSERT(( CopyConstructibleConcept<Visitor> ));
      vis.initialize_vertex(u, g);
      vis.start_vertex(u, g);
      vis.discover_vertex(u, g);
      vis.examine_edge(e, g);
      vis.tree_edge(e, g);
      vis.back_edge(e, g);
      vis.forward_or_cross_edge(e, g);
      // vis.finish_edge(e, g); // Optional for user
      vis.finish_vertex(u, g);
    }
  private:
    Visitor vis;
    Graph g;
    typename graph_traits<Graph>::vertex_descriptor u;
    typename graph_traits<Graph>::edge_descriptor e;
  };

  namespace detail {

    struct nontruth2 {
      template<class T, class T2>
      bool operator()(const T&, const T2&) const { return false; }
    };

    template <bool IsCallable> struct do_call_finish_edge {
      template <typename E, typename G, typename Vis>
      static void call_finish_edge(Vis& vis, E e, const G& g) {
        vis.finish_edge(e, g);
      }
    };

    template <> struct do_call_finish_edge<false> {
      template <typename E, typename G, typename Vis>
      static void call_finish_edge(Vis&, E, const G&) {}
    };

    template <typename E, typename G, typename Vis>
    void call_finish_edge(Vis& vis, E e, const G& g) { // Only call if method exists
      do_call_finish_edge<
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS) && ( \
    !defined(BOOST_NO_CXX11_DECLTYPE) || defined(BOOST_TYPEOF_KEYWORD) \
)
        is_dfs_visitor_with_finish_edge<Vis,G>::value
#else
        false
#endif
      >::call_finish_edge(vis, e, g);
    }


// Define BOOST_RECURSIVE_DFS to use older, recursive version.
// It is retained for a while in order to perform performance
// comparison.
#ifndef BOOST_RECURSIVE_DFS

    // If the vertex u and the iterators ei and ei_end are thought of as the
    // context of the algorithm, each push and pop from the stack could
    // be thought of as a context shift.
    // Each pass through "while (ei != ei_end)" may refer to the out-edges of
    // an entirely different vertex, because the context of the algorithm
    // shifts every time a white adjacent vertex is discovered.
    // The corresponding context shift back from the adjacent vertex occurs
    // after all of its out-edges have been examined.
    //
    // See http://lists.boost.org/MailArchives/boost/msg48752.php for FAQ.

    template <class IncidenceGraph, class DFSVisitor, class ColorMap,
            class TerminatorFunc>
    void depth_first_visit_impl
      (const IncidenceGraph& g,
       typename graph_traits<IncidenceGraph>::vertex_descriptor u,
       DFSVisitor& vis,
       ColorMap color, TerminatorFunc func = TerminatorFunc())
    {
      BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<IncidenceGraph> ));
      BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, IncidenceGraph> ));
      typedef typename graph_traits<IncidenceGraph>::vertex_descriptor Vertex;
      typedef typename graph_traits<IncidenceGraph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<ColorMap, Vertex> ));
      typedef typename property_traits<ColorMap>::value_type ColorValue;
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<ColorValue> ));
      typedef color_traits<ColorValue> Color;
      typedef typename graph_traits<IncidenceGraph>::out_edge_iterator Iter;
      typedef std::pair<Vertex, std::pair<boost::optional<Edge>, std::pair<Iter, Iter> > > VertexInfo;

      boost::optional<Edge> src_e;
      Iter ei, ei_end;
      std::vector<VertexInfo> stack;

      // Possible optimization for vector
      //stack.reserve(num_vertices(g));

      put(color, u, Color::gray());
      vis.discover_vertex(u, g);
      boost::tie(ei, ei_end) = out_edges(u, g);
      if (func(u, g)) {
          // If this vertex terminates the search, we push empty range
          stack.push_back(std::make_pair(u, std::make_pair(boost::optional<Edge>(), std::make_pair(ei_end, ei_end))));
      } else {
          stack.push_back(std::make_pair(u, std::make_pair(boost::optional<Edge>(), std::make_pair(ei, ei_end))));
      }
      while (!stack.empty()) {
        VertexInfo& back = stack.back();
        u = back.first;
        src_e = back.second.first;
        boost::tie(ei, ei_end) = back.second.second;
        stack.pop_back();
	// finish_edge has to be called here, not after the
	// loop. Think of the pop as the return from a recursive call.
        if (src_e) {
	  call_finish_edge(vis, src_e.get(), g);
	}
        while (ei != ei_end) {
          Vertex v = target(*ei, g);
          vis.examine_edge(*ei, g);
          ColorValue v_color = get(color, v);
          if (v_color == Color::white()) {
            vis.tree_edge(*ei, g);
            src_e = *ei;
            stack.push_back(std::make_pair(u, std::make_pair(src_e, std::make_pair(++ei, ei_end))));
            u = v;
            put(color, u, Color::gray());
            vis.discover_vertex(u, g);
            boost::tie(ei, ei_end) = out_edges(u, g);
            if (func(u, g)) {
                ei = ei_end;
            }
          } else {
            if (v_color == Color::gray()) {
              vis.back_edge(*ei, g);
            } else {
              vis.forward_or_cross_edge(*ei, g);
            }
            call_finish_edge(vis, *ei, g);
            ++ei;
          }
        }
        put(color, u, Color::black());
        vis.finish_vertex(u, g);
      }
    }

#else // BOOST_RECURSIVE_DFS is defined

    template <class IncidenceGraph, class DFSVisitor, class ColorMap,
              class TerminatorFunc>
    void depth_first_visit_impl
      (const IncidenceGraph& g,
       typename graph_traits<IncidenceGraph>::vertex_descriptor u,
       DFSVisitor& vis,  // pass-by-reference here, important!
       ColorMap color, TerminatorFunc func)
    {
      BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<IncidenceGraph> ));
      BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, IncidenceGraph> ));
      typedef typename graph_traits<IncidenceGraph>::vertex_descriptor Vertex;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<ColorMap, Vertex> ));
      typedef typename property_traits<ColorMap>::value_type ColorValue;
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<ColorValue> ));
      typedef color_traits<ColorValue> Color;
      typename graph_traits<IncidenceGraph>::out_edge_iterator ei, ei_end;

      put(color, u, Color::gray());          vis.discover_vertex(u, g);

      if (!func(u, g))
        for (boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
          Vertex v = target(*ei, g);           vis.examine_edge(*ei, g);
          ColorValue v_color = get(color, v);
          if (v_color == Color::white()) {     vis.tree_edge(*ei, g);
          depth_first_visit_impl(g, v, vis, color, func);
          } else if (v_color == Color::gray()) vis.back_edge(*ei, g);
          else                                 vis.forward_or_cross_edge(*ei, g);
          call_finish_edge(vis, *ei, g);
        }
      put(color, u, Color::black());         vis.finish_vertex(u, g);
    }

#endif

  } // namespace detail

#if !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <class VertexListGraph, class DFSVisitor, class ColorMap>
  void
  depth_first_search(const VertexListGraph& g, DFSVisitor vis, ColorMap color,
                     typename graph_traits<VertexListGraph>::vertex_descriptor start_vertex)
  {
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
    BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, VertexListGraph> ));
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;

    typename graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      Vertex u = implicit_cast<Vertex>(*ui);
      put(color, u, Color::white()); vis.initialize_vertex(u, g);
    }

    if (start_vertex != detail::get_default_starting_vertex(g)){ vis.start_vertex(start_vertex, g);
      detail::depth_first_visit_impl(g, start_vertex, vis, color,
                                     detail::nontruth2());
    }

    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      Vertex u = implicit_cast<Vertex>(*ui);
      ColorValue u_color = get(color, u);
      if (u_color == Color::white()) {       vis.start_vertex(u, g);
        detail::depth_first_visit_impl(g, u, vis, color, detail::nontruth2());
      }
    }
  }

  template <class VertexListGraph, class DFSVisitor, class ColorMap>
  void
  depth_first_search(const VertexListGraph& g, DFSVisitor vis, ColorMap color)
  {
    typedef typename boost::graph_traits<VertexListGraph>::vertex_iterator vi;
    std::pair<vi, vi> verts = vertices(g);
    if (verts.first == verts.second)
      return;

    depth_first_search(g, vis, color, detail::get_default_starting_vertex(g));
  }
#endif  // !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)

  template <class Visitors = null_visitor>
  class dfs_visitor {
  public:
    dfs_visitor() : m_vis() { }
    dfs_visitor(Visitors vis) : m_vis(vis) { }

    template <class Vertex, class Graph>
    void initialize_vertex(Vertex u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_initialize_vertex());
    }
    template <class Vertex, class Graph>
    void start_vertex(Vertex u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_start_vertex());
    }
    template <class Vertex, class Graph>
    void discover_vertex(Vertex u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_discover_vertex());
    }
    template <class Edge, class Graph>
    void examine_edge(Edge u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_examine_edge());
    }
    template <class Edge, class Graph>
    void tree_edge(Edge u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_tree_edge());
    }
    template <class Edge, class Graph>
    void back_edge(Edge u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_back_edge());
    }
    template <class Edge, class Graph>
    void forward_or_cross_edge(Edge u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_forward_or_cross_edge());
    }
    template <class Edge, class Graph>
    void finish_edge(Edge u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_finish_edge());
    }
    template <class Vertex, class Graph>
    void finish_vertex(Vertex u, const Graph& g) {
      invoke_visitors(m_vis, u, g, ::boost::on_finish_vertex());
    }

    BOOST_GRAPH_EVENT_STUB(on_initialize_vertex,dfs)
    BOOST_GRAPH_EVENT_STUB(on_start_vertex,dfs)
    BOOST_GRAPH_EVENT_STUB(on_discover_vertex,dfs)
    BOOST_GRAPH_EVENT_STUB(on_examine_edge,dfs)
    BOOST_GRAPH_EVENT_STUB(on_tree_edge,dfs)
    BOOST_GRAPH_EVENT_STUB(on_back_edge,dfs)
    BOOST_GRAPH_EVENT_STUB(on_forward_or_cross_edge,dfs)
    BOOST_GRAPH_EVENT_STUB(on_finish_edge,dfs)
    BOOST_GRAPH_EVENT_STUB(on_finish_vertex,dfs)

  protected:
    Visitors m_vis;
  };
  template <class Visitors>
  dfs_visitor<Visitors>
  make_dfs_visitor(Visitors vis) {
    return dfs_visitor<Visitors>(vis);
  }
  typedef dfs_visitor<> default_dfs_visitor;

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  // Boost.Parameter-enabled variant
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), depth_first_search, ::boost::graph::keywords::tag,
    (required
      (graph, *(detail::argument_predicate<is_vertex_list_graph>))
    )
    (deduced
      (optional
        (vertex_index_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_to_integer_map_of_graph
            >
          )
          ,detail::vertex_or_dummy_property_map(graph, vertex_index)
        )
        (color_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_color_map_of_graph
            >
          )
          ,make_shared_array_property_map(
            num_vertices(graph),
            white_color,
            vertex_index_map
          )
        )
        (visitor
          ,*(detail::dfs_visitor_predicate)
          ,default_dfs_visitor()
        )
        (root_vertex
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_of_graph
            >
          )
          ,detail::get_default_starting_vertex(graph)
        )
      )
    )
  )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (
      boost::disable_if<
        detail::is_bgl_named_param_argument<
          Args,
          boost::graph::keywords::tag::visitor
        >,
        bool
      >
    ), depth_first_search, ::boost::graph::keywords::tag,
    (required
      (graph, *)
    )
    (optional
      (visitor, *, default_dfs_visitor())
      (color_map
        ,*
        ,make_shared_array_property_map(
          num_vertices(graph),
          white_color,
          detail::vertex_or_dummy_property_map(graph, vertex_index)
        )
      )
      (root_vertex, *, detail::get_default_starting_vertex(graph))
    )
  )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
  {
    typedef typename boost::remove_const<
      typename boost::remove_reference<graph_type>::type
    >::type VertexListGraph;
    typedef typename boost::remove_const<
      typename boost::remove_reference<visitor_type>::type
    >::type DFSVisitor;
    BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, VertexListGraph> ));
    typedef typename boost::remove_const<
      typename boost::remove_reference<color_map_type>::type
    >::type ColorMap;
    typedef typename graph_traits<VertexListGraph>::vertex_descriptor Vertex;
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;

    DFSVisitor vis = visitor;
    typename graph_traits<VertexListGraph>::vertex_iterator ui, ui_end;

    for (boost::tie(ui, ui_end) = vertices(graph); ui != ui_end; ++ui) {
      Vertex u = implicit_cast<Vertex>(*ui);
      put(color_map, u, Color::white());
      vis.initialize_vertex(u, graph);
    }

    Vertex s = root_vertex;

    if (s != detail::get_default_starting_vertex(graph)) {
      vis.start_vertex(s, graph);
      detail::depth_first_visit_impl(graph, s, vis, color_map,
                                     detail::nontruth2());
    }

    for (boost::tie(ui, ui_end) = vertices(graph); ui != ui_end; ++ui) {
      Vertex u = implicit_cast<Vertex>(*ui);
      ColorValue u_color = get(color_map, u);
      if (u_color == Color::white()) {
        vis.start_vertex(u, graph);
        detail::depth_first_visit_impl(graph, u, vis, color_map,
                                       detail::nontruth2());
      }
    }

    return true;
  }

  // Old-style named parameter variant
  template <typename Graph, typename P, typename T, typename R>
  void depth_first_search(const Graph& g,
                          const bgl_named_params<P, T, R>& params)
  {
    typedef bgl_named_params<P, T, R> params_type;
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)
    depth_first_search(
      g,
      boost::graph::keywords::_visitor = arg_pack[
        boost::graph::keywords::_visitor ||
        boost::value_factory<default_dfs_visitor>()
      ],
      boost::graph::keywords::_color_map = arg_pack[
        boost::graph::keywords::_color_map |
        make_shared_array_property_map(
          num_vertices(g),
          white_color,
          arg_pack[
            boost::graph::keywords::_vertex_index_map |
            detail::vertex_or_dummy_property_map(g, vertex_index)
          ]
        )
      ],
      boost::graph::keywords::_root_vertex = arg_pack[
        boost::graph::keywords::_root_vertex ||
        boost::detail::get_default_starting_vertex_t<Graph>(g)
      ]
    );
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), depth_first_visit, ::boost::graph::keywords::tag,
    (required
      (graph, *(detail::argument_predicate<is_incidence_graph>))
    )
    (deduced
      (required
        (root_vertex
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_of_graph
            >
          )
        )
        (visitor, *(detail::dfs_visitor_predicate))
        (color_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_vertex_color_map_of_graph
            >
          )
        )
      )
      (optional
        (terminator_function
          ,*(detail::binary_function_graph_predicate)
          ,detail::nontruth2()
        )
      )
    )
  )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), depth_first_visit, ::boost::graph::keywords::tag,
    (required
      (graph, *)
      (root_vertex, *)
      (visitor, *)
      (color_map, *)
    )
    (optional
      (terminator_function, *, detail::nontruth2())
    )
  )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
  {
    typename boost::remove_const<
      typename boost::remove_reference<root_vertex_type>::type
    >::type s = root_vertex;
    typename boost::remove_const<
      typename boost::remove_reference<visitor_type>::type
    >::type vis = visitor;
    vis.start_vertex(s, graph);
    detail::depth_first_visit_impl(graph, s, vis, color_map,
                                   terminator_function);
    return true;
  }
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  // Boost.Parameter named parameter variant
  namespace graph {
    namespace detail {
      template <typename Graph>
      struct depth_first_search_impl {
        typedef void result_type;
        template <typename ArgPack>
        void operator()(const Graph& g, const ArgPack& arg_pack) const {
          using namespace boost::graph::keywords;
          boost::depth_first_search(g,
                                    arg_pack[_visitor | make_dfs_visitor(null_visitor())],
                                    boost::detail::make_color_map_from_arg_pack(g, arg_pack),
                                    arg_pack[_root_vertex || boost::detail::get_default_starting_vertex_t<Graph>(g)]);
        }
      };
    }
    BOOST_GRAPH_MAKE_FORWARDING_FUNCTION(depth_first_search, 1, 4)
  }

  BOOST_GRAPH_MAKE_OLD_STYLE_PARAMETER_FUNCTION(depth_first_search, 1)

  template <class IncidenceGraph, class DFSVisitor, class ColorMap>
  void depth_first_visit
    (const IncidenceGraph& g,
     typename graph_traits<IncidenceGraph>::vertex_descriptor u,
     DFSVisitor vis, ColorMap color)
  {
    vis.start_vertex(u, g);
    detail::depth_first_visit_impl(g, u, vis, color, detail::nontruth2());
  }

  template <class IncidenceGraph, class DFSVisitor, class ColorMap,
            class TerminatorFunc>
  void depth_first_visit
    (const IncidenceGraph& g,
     typename graph_traits<IncidenceGraph>::vertex_descriptor u,
     DFSVisitor vis, ColorMap color, TerminatorFunc func = TerminatorFunc())
  {
    vis.start_vertex(u, g);
    detail::depth_first_visit_impl(g, u, vis, color, func);
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS
} // namespace boost

#include BOOST_GRAPH_MPI_INCLUDE(<boost/graph/distributed/depth_first_search.hpp>)

#endif
