//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_TRAITS_HPP
#define BOOST_GRAPH_TRAITS_HPP

#include <boost/config.hpp>
#include <iterator>
#include <utility> /* Primarily for std::pair */
#include <boost/tuple/tuple.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/has_xxx.hpp>
#include <boost/mpl/void.hpp>
#include <boost/mpl/identity.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/iterator/iterator_categories.hpp>
#include <boost/iterator/iterator_adaptor.hpp>
#include <boost/pending/property.hpp>
#include <boost/detail/workaround.hpp>

namespace boost {
    namespace graph_detail {
        BOOST_MPL_HAS_XXX_TRAIT_DEF(vertex_descriptor)
        BOOST_MPL_HAS_XXX_TRAIT_DEF(edge_descriptor)
        BOOST_MPL_HAS_XXX_TRAIT_DEF(directed_category)
        BOOST_MPL_HAS_XXX_TRAIT_DEF(edge_parallel_category)
        BOOST_MPL_HAS_XXX_TRAIT_DEF(traversal_category)
    }

    namespace detail {
        template <typename T>
        struct has_graph_typedefs
            : mpl::eval_if<
                graph_detail::has_vertex_descriptor<T>,
                mpl::eval_if<
                    graph_detail::has_edge_descriptor<T>,
                    mpl::eval_if<
                        graph_detail::has_directed_category<T>,
                        mpl::if_<
                            graph_detail::has_edge_parallel_category<T>,
                            graph_detail::has_traversal_category<T>,
                            mpl::false_
                        >,
                        mpl::false_
                    >,
                    mpl::false_
                >,
                mpl::false_
            >::type
        { };

#define BOOST_GRAPH_MEMBER_OR_VOID(name) \
      BOOST_MPL_HAS_XXX_TRAIT_DEF(name) \
      template <typename T> struct BOOST_JOIN(get_member_, name) {typedef typename T::name type;}; \
      template <typename T> struct BOOST_JOIN(get_opt_member_, name): \
        boost::mpl::eval_if_c< \
          BOOST_JOIN(has_, name)<T>::value, \
          BOOST_JOIN(get_member_, name)<T>, \
          boost::mpl::identity<void> > \
        {};
      BOOST_GRAPH_MEMBER_OR_VOID(adjacency_iterator)
      BOOST_GRAPH_MEMBER_OR_VOID(out_edge_iterator)
      BOOST_GRAPH_MEMBER_OR_VOID(in_edge_iterator)
      BOOST_GRAPH_MEMBER_OR_VOID(vertex_iterator)
      BOOST_GRAPH_MEMBER_OR_VOID(edge_iterator)
      BOOST_GRAPH_MEMBER_OR_VOID(vertices_size_type)
      BOOST_GRAPH_MEMBER_OR_VOID(edges_size_type)
      BOOST_GRAPH_MEMBER_OR_VOID(degree_size_type)
#undef BOOST_GRAPH_MEMBER_OR_VOID

        template <typename G>
        struct graph_traits_impl {
#define BOOST_GRAPH_PULL_OPT_MEMBER(name) \
            typedef typename detail::BOOST_JOIN(get_opt_member_, name)< \
                G \
            >::type name;

            typedef typename G::vertex_descriptor      vertex_descriptor;
            typedef typename G::edge_descriptor        edge_descriptor;
            BOOST_GRAPH_PULL_OPT_MEMBER(adjacency_iterator)
            BOOST_GRAPH_PULL_OPT_MEMBER(out_edge_iterator)
            BOOST_GRAPH_PULL_OPT_MEMBER(in_edge_iterator)
            BOOST_GRAPH_PULL_OPT_MEMBER(vertex_iterator)
            BOOST_GRAPH_PULL_OPT_MEMBER(edge_iterator)

            typedef typename G::directed_category      directed_category;
            typedef typename G::edge_parallel_category edge_parallel_category;
            typedef typename G::traversal_category     traversal_category;

            BOOST_GRAPH_PULL_OPT_MEMBER(vertices_size_type)
            BOOST_GRAPH_PULL_OPT_MEMBER(edges_size_type)
            BOOST_GRAPH_PULL_OPT_MEMBER(degree_size_type)
#undef BOOST_GRAPH_PULL_OPT_MEMBER

            static vertex_descriptor null_vertex();
        };

        template <typename G>
        inline typename graph_traits_impl<G>::vertex_descriptor
        graph_traits_impl<G>::null_vertex()
        { return G::null_vertex(); }

        struct graph_traits_no_impl { };
    }

    // The primary template definition is SFINAE-friendly so that is_bgl_graph and
    // its relatives can be used for template argument deduction.
    // -- Cromwell D. Enage
    template <typename G>
    struct graph_traits
        : mpl::if_<
            detail::has_graph_typedefs<G>,
            detail::graph_traits_impl<G>,
            detail::graph_traits_no_impl
        >::type
    { };

    template <typename T>
    struct is_bgl_graph : detail::has_graph_typedefs< graph_traits<T> >::type
    { };

    // directed_category tags
    struct directed_tag { };
    struct undirected_tag { };
    struct bidirectional_tag : public directed_tag { };

    namespace detail {
        inline bool is_directed(directed_tag) { return true; }
        inline bool is_directed(undirected_tag) { return false; }
    }

    /** Return true if the given graph is directed. */
    template <typename Graph>
    bool is_directed(const Graph&) {
        typedef typename graph_traits<Graph>::directed_category Cat;
        return detail::is_directed(Cat());
    }

    /** Return true if the given graph is undirected. */
    template <typename Graph>
    bool is_undirected(const Graph& g) {
        return !is_directed(g);
    }

    namespace graph_detail {
        template <typename Tag>
        struct is_directed_tag
            : mpl::bool_<is_convertible<Tag, directed_tag>::value>
        { };
    } // namespace graph_detail

    namespace detail {
        template <typename Graph>
        struct is_directed_graph_impl
            : graph_detail::is_directed_tag<
                typename graph_traits<Graph>::directed_category
            >
        { };
    }

    /** @name Directed/Undirected Graph Traits */
    //@{
    template <typename Graph>
    struct is_directed_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_directed_graph_impl<Graph>,
            mpl::false_
        >::type
    { };

    template <typename Graph>
    struct is_undirected_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            mpl::not_< is_directed_graph<Graph> >,
            mpl::false_
        >::type
    { };
    //@}

    // edge_parallel_category tags
    struct allow_parallel_edge_tag { };
    struct disallow_parallel_edge_tag { };

    namespace detail {
        inline bool allows_parallel(allow_parallel_edge_tag) { return true; }
        inline bool allows_parallel(disallow_parallel_edge_tag) { return false; }
    }

    template <typename Graph>
    bool allows_parallel_edges(const Graph&) {
        typedef typename graph_traits<Graph>::edge_parallel_category Cat;
        return detail::allows_parallel(Cat());
    }

    namespace detail {
        template <typename Graph>
        struct is_multigraph_impl
            : mpl::bool_<
                is_same<
                    typename graph_traits<Graph>::edge_parallel_category,
                    allow_parallel_edge_tag
                >::value
            >
        { };
    }

    /** @name Parallel Edges Traits */
    //@{
    /**
     * The is_multigraph metafunction returns true if the graph allows
     * parallel edges. Technically, a multigraph is a simple graph that
     * allows parallel edges, but since there are no traits for the allowance
     * or disallowance of loops, this is a moot point.
     */
    template <typename Graph>
    struct is_multigraph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_multigraph_impl<Graph>,
            mpl::false_
        >::type
    { };
    //@}

    // traversal_category tags
    struct incidence_graph_tag { };
    struct adjacency_graph_tag { };
    struct bidirectional_graph_tag : virtual incidence_graph_tag { };
    struct vertex_list_graph_tag { };
    struct edge_list_graph_tag { };
    struct adjacency_matrix_tag { };

    // Parallel traversal_category tags
    struct distributed_graph_tag { };
    struct distributed_vertex_list_graph_tag { };
    struct distributed_edge_list_graph_tag { };
#define BOOST_GRAPH_SEQUENTIAL_TRAITS_DEFINES_DISTRIBUTED_TAGS // Disable these from external versions of PBGL

    namespace detail {
        template <typename Graph>
        struct is_incidence_graph_impl
            : mpl::bool_<
                is_convertible<
                    typename graph_traits<Graph>::traversal_category,
                    incidence_graph_tag
                >::value
            >
        { };
        template <typename Graph>
        struct is_bidirectional_graph_impl
            : mpl::bool_<
                is_convertible<
                    typename graph_traits<Graph>::traversal_category,
                    bidirectional_graph_tag
                >::value
            >
        { };
        template <typename Graph>
        struct is_vertex_list_graph_impl
            : mpl::bool_<
                is_convertible<
                    typename graph_traits<Graph>::traversal_category,
                    vertex_list_graph_tag
                >::value
            >
        { };
        template <typename Graph>
        struct is_edge_list_graph_impl
            : mpl::bool_<
                is_convertible<
                    typename graph_traits<Graph>::traversal_category,
                    edge_list_graph_tag
                >::value
            >
        { };
        template <typename Graph>
        struct is_adjacency_matrix_impl
            : mpl::bool_<
                is_convertible<
                    typename graph_traits<Graph>::traversal_category,
                    adjacency_matrix_tag
                >::value
            >
        { };
    }

    /** @name Traversal Category Traits
     * These traits classify graph types by their supported methods of
     * vertex and edge traversal.
     */
    //@{
    template <typename Graph>
    struct is_incidence_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_incidence_graph_impl<Graph>,
            mpl::false_
        >::type
    { };

    template <typename Graph>
    struct is_bidirectional_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_bidirectional_graph_impl<Graph>,
            mpl::false_
        >::type
    { };

    template <typename Graph>
    struct is_vertex_list_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_vertex_list_graph_impl<Graph>,
            mpl::false_
        >::type
    { };

    template <typename Graph>
    struct is_edge_list_graph
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_edge_list_graph_impl<Graph>,
            mpl::false_
        >::type
    { };

    template <typename Graph>
    struct is_adjacency_matrix
        : mpl::if_<
            is_bgl_graph<Graph>,
            detail::is_adjacency_matrix_impl<Graph>,
            mpl::false_
        >::type
    { };
    //@}

    namespace detail {
        template <typename Vertex, typename Graph>
        struct is_vertex_of_graph_impl
            : mpl::bool_<
                is_same<
                    Vertex,
                    typename graph_traits<Graph>::vertex_descriptor
                >::value
            >
        { };

        template <typename Vertex, typename Graph>
        struct is_vertex_of_graph
            : mpl::if_<
                is_bgl_graph<Graph>,
                detail::is_vertex_of_graph_impl<Vertex, Graph>,
                mpl::false_
            >::type
        { };
    }

    /** @name Directed Graph Traits
     * These metafunctions are used to fully classify directed vs. undirected
     * graphs. Recall that an undirected graph is also bidirectional, but it
     * cannot be both undirected and directed at the same time.
     */
    //@{
    template <typename Graph>
    struct is_directed_unidirectional_graph
        : mpl::and_<
            is_directed_graph<Graph>, mpl::not_< is_bidirectional_graph<Graph> >
        >
    { };

    template <typename Graph>
    struct is_directed_bidirectional_graph
        : mpl::and_<
            is_directed_graph<Graph>, is_bidirectional_graph<Graph>
        >
    { };
    //@}

    //?? not the right place ?? Lee
    typedef boost::forward_traversal_tag multi_pass_input_iterator_tag;

    namespace detail {
      BOOST_MPL_HAS_XXX_TRAIT_DEF(graph_property_type)
      BOOST_MPL_HAS_XXX_TRAIT_DEF(edge_property_type)
      BOOST_MPL_HAS_XXX_TRAIT_DEF(vertex_property_type)

      template <typename G> struct get_graph_property_type {typedef typename G::graph_property_type type;};
      template <typename G> struct get_edge_property_type {typedef typename G::edge_property_type type;};
      template <typename G> struct get_vertex_property_type {typedef typename G::vertex_property_type type;};
    }

    template <typename G>
    struct graph_property_type
      : boost::mpl::eval_if<detail::has_graph_property_type<G>,
                            detail::get_graph_property_type<G>,
                            no_property> {};
    template <typename G>
    struct edge_property_type
      : boost::mpl::eval_if<detail::has_edge_property_type<G>,
                            detail::get_edge_property_type<G>,
                            no_property> {};
    template <typename G>
    struct vertex_property_type
      : boost::mpl::eval_if<detail::has_vertex_property_type<G>,
                            detail::get_vertex_property_type<G>,
                            no_property> {};

    template<typename G>
    struct graph_bundle_type {
      typedef typename G::graph_bundled type;
    };

    template<typename G>
    struct vertex_bundle_type {
      typedef typename G::vertex_bundled type;
    };

    template<typename G>
    struct edge_bundle_type {
      typedef typename G::edge_bundled type;
    };

    namespace graph { namespace detail {
        template<typename Graph, typename Descriptor>
        class bundled_result {
            typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
            typedef typename mpl::if_c<(is_same<Descriptor, Vertex>::value),
                                        vertex_bundle_type<Graph>,
                                        edge_bundle_type<Graph> >::type bundler;
        public:
            typedef typename bundler::type type;
        };

        template<typename Graph>
        class bundled_result<Graph, graph_bundle_t> {
            typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
            typedef graph_bundle_type<Graph> bundler;
        public:
            typedef typename bundler::type type;
        };

    } } // namespace graph::detail

    namespace graph_detail {
      // A helper metafunction for determining whether or not a type is
      // bundled.
      template <typename T>
      struct is_no_bundle : mpl::bool_<is_same<T, no_property>::value>
      { };
    } // namespace graph_detail

    /** @name Graph Property Traits
     * These metafunctions (along with those above), can be used to access the
     * vertex and edge properties (bundled or otherwise) of vertices and
     * edges.
     */
    //@{
    template<typename Graph>
    struct has_graph_property
      : mpl::not_<
        typename detail::is_no_property<
          typename graph_property_type<Graph>::type
        >::type
      >::type
    { };

    template<typename Graph>
    struct has_bundled_graph_property
      : mpl::not_<
        graph_detail::is_no_bundle<typename graph_bundle_type<Graph>::type>
      >
    { };

    template <typename Graph>
    struct has_vertex_property
        : mpl::not_<
            typename detail::is_no_property<typename vertex_property_type<Graph>::type>
        >::type
    { };

    template <typename Graph>
    struct has_bundled_vertex_property
        : mpl::not_<
            graph_detail::is_no_bundle<typename vertex_bundle_type<Graph>::type>
        >
    { };

    template <typename Graph>
    struct has_edge_property
        : mpl::not_<
            typename detail::is_no_property<typename edge_property_type<Graph>::type>
        >::type
    { };

    template <typename Graph>
    struct has_bundled_edge_property
        : mpl::not_<
            graph_detail::is_no_bundle<typename edge_bundle_type<Graph>::type>
        >
    { };
    //@}

} // namespace boost

// Since pair is in namespace std, Koenig lookup will find source and
// target if they are also defined in namespace std.  This is illegal,
// but the alternative is to put source and target in the global
// namespace which causes name conflicts with other libraries (like
// SUIF).
namespace std {

  /* Some helper functions for dealing with pairs as edges */
  template <class T, class G>
  T source(pair<T,T> p, const G&) { return p.first; }

  template <class T, class G>
  T target(pair<T,T> p, const G&) { return p.second; }

}

#if defined(__GNUC__) && defined(__SGI_STL_PORT)
// For some reason g++ with STLport does not see the above definition
// of source() and target() unless we bring them into the boost
// namespace.
namespace boost {
  using std::source;
  using std::target;
}
#endif

#endif // BOOST_GRAPH_TRAITS_HPP
