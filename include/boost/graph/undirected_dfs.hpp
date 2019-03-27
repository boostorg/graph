//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
#ifndef BOOST_GRAPH_UNDIRECTED_DFS_HPP
#define BOOST_GRAPH_UNDIRECTED_DFS_HPP

#include <boost/graph/depth_first_search.hpp>
#include <vector>
#include <boost/concept/assert.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/placeholders.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/if.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS) && \
    !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/parameter/are_tagged_arguments.hpp>
#include <boost/parameter/is_argument_pack.hpp>
#include <boost/parameter/compose.hpp>
#include <boost/core/enable_if.hpp>
#include <boost/preprocessor/repetition/enum_trailing_binary_params.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#include <boost/preprocessor/repetition/repeat_from_to.hpp>
#endif

namespace boost {

  namespace detail {

// Define BOOST_RECURSIVE_DFS to use older, recursive version.
// It is retained for a while in order to perform performance
// comparison.
#ifndef BOOST_RECURSIVE_DFS

    template <typename IncidenceGraph, typename DFSVisitor, 
              typename VertexColorMap, typename EdgeColorMap>
    void undir_dfv_impl
      (const IncidenceGraph& g,
       typename graph_traits<IncidenceGraph>::vertex_descriptor u, 
       DFSVisitor& vis,
       VertexColorMap vertex_color,
       EdgeColorMap edge_color)
    {
      BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<IncidenceGraph> ));
      BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, IncidenceGraph> ));
      typedef typename graph_traits<IncidenceGraph>::vertex_descriptor Vertex;
      typedef typename graph_traits<IncidenceGraph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<VertexColorMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<EdgeColorMap,Edge> ));
      typedef typename property_traits<VertexColorMap>::value_type ColorValue;
      typedef typename property_traits<EdgeColorMap>::value_type EColorValue;
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<ColorValue> ));
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<EColorValue> ));
      typedef color_traits<ColorValue> Color;
      typedef color_traits<EColorValue> EColor;
      typedef typename graph_traits<IncidenceGraph>::out_edge_iterator Iter;
      typedef std::pair<Vertex, std::pair<boost::optional<Edge>, std::pair<Iter, Iter> > > VertexInfo;

      std::vector<VertexInfo> stack;

      put(vertex_color, u, Color::gray());
      vis.discover_vertex(u, g);
      stack.push_back(std::make_pair(u, std::make_pair(boost::optional<Edge>(), out_edges(u, g))));
      while (!stack.empty()) {
        VertexInfo& back = stack.back();
        u = back.first;
        boost::optional<Edge> src_e = back.second.first;
        Iter ei = back.second.second.first, ei_end = back.second.second.second;
        stack.pop_back();
        while (ei != ei_end) {
          Vertex v = target(*ei, g);
          vis.examine_edge(*ei, g);
          ColorValue v_color = get(vertex_color, v);
          EColorValue uv_color = get(edge_color, *ei);
          put(edge_color, *ei, EColor::black());
          if (v_color == Color::white()) {
            vis.tree_edge(*ei, g);
            src_e = *ei;
            stack.push_back(std::make_pair(u, std::make_pair(src_e, std::make_pair(++ei, ei_end))));
            u = v;
            put(vertex_color, u, Color::gray());
            vis.discover_vertex(u, g);
            boost::tie(ei, ei_end) = out_edges(u, g);
          } else if (v_color == Color::gray()) {
            if (uv_color == EColor::white()) vis.back_edge(*ei, g);
            call_finish_edge(vis, *ei, g);
            ++ei;
          } else { // if (v_color == Color::black())
            call_finish_edge(vis, *ei, g);
            ++ei;
          }
        }
        put(vertex_color, u, Color::black());
        vis.finish_vertex(u, g);
        if (src_e) call_finish_edge(vis, src_e.get(), g);
      }
    }

#else // BOOST_RECURSIVE_DFS

    template <typename IncidenceGraph, typename DFSVisitor, 
              typename VertexColorMap, typename EdgeColorMap>
    void undir_dfv_impl
      (const IncidenceGraph& g,
       typename graph_traits<IncidenceGraph>::vertex_descriptor u, 
       DFSVisitor& vis,  // pass-by-reference here, important!
       VertexColorMap vertex_color,
       EdgeColorMap edge_color)
    {
      BOOST_CONCEPT_ASSERT(( IncidenceGraphConcept<IncidenceGraph> ));
      BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, IncidenceGraph> ));
      typedef typename graph_traits<IncidenceGraph>::vertex_descriptor Vertex;
      typedef typename graph_traits<IncidenceGraph>::edge_descriptor Edge;
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<VertexColorMap,Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadWritePropertyMapConcept<EdgeColorMap,Edge> ));
      typedef typename property_traits<VertexColorMap>::value_type ColorValue;
      typedef typename property_traits<EdgeColorMap>::value_type EColorValue;
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<ColorValue> ));
      BOOST_CONCEPT_ASSERT(( ColorValueConcept<EColorValue> ));
      typedef color_traits<ColorValue> Color;
      typedef color_traits<EColorValue> EColor;
      typename graph_traits<IncidenceGraph>::out_edge_iterator ei, ei_end;

      put(vertex_color, u, Color::gray());   vis.discover_vertex(u, g);
      for (boost::tie(ei, ei_end) = out_edges(u, g); ei != ei_end; ++ei) {
        Vertex v = target(*ei, g);           vis.examine_edge(*ei, g);
        ColorValue v_color = get(vertex_color, v);
        EColorValue uv_color = get(edge_color, *ei);
        put(edge_color, *ei, EColor::black());
        if (v_color == Color::white()) {     vis.tree_edge(*ei, g);
          undir_dfv_impl(g, v, vis, vertex_color, edge_color);
        } else if (v_color == Color::gray() && uv_color == EColor::white())
                                             vis.back_edge(*ei, g);
                                             call_finish_edge(vis, *ei, g);
      }
      put(vertex_color, u, Color::black());  vis.finish_vertex(u, g);
    }

#endif // ! BOOST_RECURSIVE_DFS

  } // namespace detail

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  // Boost.Parameter-enabled variant
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), undirected_dfs, ::boost::graph::keywords::tag,
    (required
      (graph, *(detail::argument_predicate<is_edge_list_graph>))
    )
    (deduced
      (required
        (edge_color_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_edge_color_map_of_graph
            >
          )
        )
      )
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
  template <typename Graph, typename visitor_type, typename color_map_type,
            typename EdgeColorMap, typename Vertex>
  void
  undirected_dfs(
    const Graph& graph, visitor_type visitor, color_map_type color_map,
    EdgeColorMap edge_color_map, Vertex root_vertex,
    typename boost::disable_if<
      parameter::are_tagged_arguments<
        visitor_type, color_map_type, EdgeColorMap, Vertex
      >,
      mpl::true_
    >::type = mpl::true_()
  )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
  {
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    typedef typename boost::remove_const<
      typename boost::remove_reference<graph_type>::type
    >::type Graph;
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
#endif
    typedef typename boost::remove_const<
      typename boost::remove_reference<visitor_type>::type
    >::type DFSVisitor;
    typedef typename boost::remove_const<
      typename boost::remove_reference<color_map_type>::type
    >::type VertexColorMap;

    BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, Graph> ));
    BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<Graph> ));

    typedef typename property_traits<VertexColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;

    DFSVisitor vis = visitor;
    typename graph_traits<Graph>::vertex_iterator ui, ui_end;

    for (boost::tie(ui, ui_end) = vertices(graph); ui != ui_end; ++ui) {
      put(color_map, *ui, Color::white());
      vis.initialize_vertex(*ui, graph);
    }

    typename graph_traits<Graph>::edge_iterator ei, ei_end;

    for (boost::tie(ei, ei_end) = edges(graph); ei != ei_end; ++ei)
      put(edge_color_map, *ei, Color::white());

    Vertex s = root_vertex;

    if (s != detail::get_default_starting_vertex(graph)) {
      vis.start_vertex(s, graph);
      detail::undir_dfv_impl(graph, s, vis, color_map, edge_color_map);
    }

    for (boost::tie(ui, ui_end) = vertices(graph); ui != ui_end; ++ui) {
      ColorValue u_color = get(color_map, *ui);
      if (u_color == Color::white()) {
        vis.start_vertex(*ui, graph);
        detail::undir_dfv_impl(graph, *ui, vis, color_map, edge_color_map);
      }
    }

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
    return true;
#endif
  }

#if !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  template <typename Graph, typename DFSVisitor, typename VertexColorMap,
    typename EdgeColorMap>
  inline void
  undirected_dfs(
    const Graph& g, DFSVisitor vis, VertexColorMap vertex_color,
    EdgeColorMap edge_color, typename boost::disable_if<
      parameter::are_tagged_arguments<
        DFSVisitor, VertexColorMap, EdgeColorMap
      >,
      mpl::true_
    >::type = mpl::true_()
  )
  {
    undirected_dfs(
      g,
      vis,
      vertex_color,
      edge_color,
      detail::get_default_starting_vertex(g)
    );
  }

  template <typename Graph, typename Args>
  inline void
  undirected_dfs(
    const Graph& g,
    const Args& arg_pack,
    typename boost::enable_if<
      parameter::is_argument_pack<Args>,
      mpl::true_
    >::type = mpl::true_()
  )
  {
    undirected_dfs(
      g,
      arg_pack[
        boost::graph::keywords::_visitor ||
        boost::value_factory<default_dfs_visitor>()
      ],
      arg_pack[
        boost::graph::keywords::_color_map |
        make_shared_array_property_map(
          num_vertices(g),
          white_color,
          arg_pack[
            boost::graph::keywords::_vertex_index_map |
            get(vertex_index, g)
          ]
        )
      ],
      arg_pack[
        boost::graph::keywords::_edge_color_map
      ],
      arg_pack[
        boost::graph::keywords::_root_vertex ||
        boost::detail::get_default_starting_vertex_t<Graph>(g)
      ]
    );
  }

#define BOOST_GRAPH_PP_FUNCTION_OVERLOAD(z, n, name) \
  template <typename Graph, typename TA \
            BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename TA)> \
  inline void name \
    (const Graph &g, const TA &ta \
     BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const TA, &ta), \
     typename boost::enable_if< \
       parameter::are_tagged_arguments< \
         TA BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, TA) \
       >, \
       mpl::true_ \
     >::type = mpl::true_()) \
  { \
    name( \
      g, \
      parameter::compose(ta BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, ta)) \
    ); \
  }

BOOST_PP_REPEAT_FROM_TO(1, 6, BOOST_GRAPH_PP_FUNCTION_OVERLOAD, undirected_dfs)

#undef BOOST_GRAPH_PP_FUNCTION_OVERLOAD

#endif  // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)

  // Old-style named parameter variant
  template <typename Graph, typename P, typename T, typename R>
  void
  undirected_dfs(const Graph& g, 
                 const bgl_named_params<P, T, R>& params)
  {
    typedef bgl_named_params<P, T, R> params_type;
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)
    undirected_dfs(
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
            get(vertex_index, g)
          ]
        )
      ],
      boost::graph::keywords::_edge_color_map = arg_pack[
        boost::graph::keywords::_edge_color_map
      ],
      boost::graph::keywords::_root_vertex = arg_pack[
        boost::graph::keywords::_root_vertex ||
        boost::detail::get_default_starting_vertex_t<Graph>(g)
      ]
    );
  }

#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), undirected_depth_first_visit, ::boost::graph::keywords::tag,
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
        (edge_color_map
          ,*(
            detail::argument_with_graph_predicate<
              detail::is_edge_color_map_of_graph
            >
          )
        )
      )
    )
  )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), undirected_depth_first_visit, ::boost::graph::keywords::tag,
    (required
      (graph, *)
      (root_vertex, *)
      (visitor, *)
      (color_map, *)
      (edge_color_map, *)
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
    detail::undir_dfv_impl(graph, s, vis, color_map, edge_color_map);
    return true;
  }
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <typename Graph, typename DFSVisitor, 
            typename VertexColorMap, typename EdgeColorMap, 
            typename Vertex>
  void
  undirected_dfs(const Graph& g, DFSVisitor vis, 
                 VertexColorMap vertex_color, EdgeColorMap edge_color,
                 Vertex start_vertex)
  {
    BOOST_CONCEPT_ASSERT(( DFSVisitorConcept<DFSVisitor, Graph> ));
    BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<Graph> ));

    typedef typename property_traits<VertexColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;

    typename graph_traits<Graph>::vertex_iterator ui, ui_end;
    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      put(vertex_color, *ui, Color::white());   vis.initialize_vertex(*ui, g);
    }
    typename graph_traits<Graph>::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
      put(edge_color, *ei, Color::white());

    if (start_vertex != *vertices(g).first){ vis.start_vertex(start_vertex, g);
      detail::undir_dfv_impl(g, start_vertex, vis, vertex_color, edge_color);
    }

    for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui) {
      ColorValue u_color = get(vertex_color, *ui);
      if (u_color == Color::white()) {       vis.start_vertex(*ui, g);
        detail::undir_dfv_impl(g, *ui, vis, vertex_color, edge_color);
      }
    }
  }

  template <typename Graph, typename DFSVisitor, typename VertexColorMap,
    typename EdgeColorMap>
  void
  undirected_dfs(const Graph& g, DFSVisitor vis, 
                 VertexColorMap vertex_color, EdgeColorMap edge_color)
  {
    undirected_dfs(g, vis, vertex_color, edge_color, *vertices(g).first);
  }

  namespace detail {
    template <typename VertexColorMap>
    struct udfs_dispatch {

      template <typename Graph, typename Vertex, 
                typename DFSVisitor, typename EdgeColorMap,
                typename P, typename T, typename R>
      static void
      apply(const Graph& g, DFSVisitor vis, Vertex start_vertex,
            const bgl_named_params<P, T, R>&,
            EdgeColorMap edge_color,
            VertexColorMap vertex_color)
      {
        undirected_dfs(g, vis, vertex_color, edge_color, start_vertex);
      }
    };

    template <>
    struct udfs_dispatch<param_not_found> {
      template <typename Graph, typename Vertex, typename DFSVisitor,
                typename EdgeColorMap,
                typename P, typename T, typename R>
      static void
      apply(const Graph& g, DFSVisitor vis, Vertex start_vertex,
            const bgl_named_params<P, T, R>& params,
            EdgeColorMap edge_color,
            param_not_found)
      {
        std::vector<default_color_type> color_vec(num_vertices(g));
        default_color_type c = white_color; // avoid warning about un-init
        undirected_dfs
          (g, vis, make_iterator_property_map
           (color_vec.begin(),
            choose_const_pmap(get_param(params, vertex_index),
                              g, vertex_index), c),
           edge_color,
           start_vertex);
      }
    };

  } // namespace detail
  

  // Named Parameter Variant
  template <typename Graph, typename P, typename T, typename R>
  void
  undirected_dfs(const Graph& g, 
                 const bgl_named_params<P, T, R>& params)
  {
    typedef typename get_param_type< vertex_color_t, bgl_named_params<P, T, R> >::type C;
    detail::udfs_dispatch<C>::apply
      (g,
       choose_param(get_param(params, graph_visitor),
                    make_dfs_visitor(null_visitor())),
       choose_param(get_param(params, root_vertex_t()),
                    *vertices(g).first),
       params,
       get_param(params, edge_color),
       get_param(params, vertex_color)
       );
  }
  

  template <typename IncidenceGraph, typename DFSVisitor, 
    typename VertexColorMap, typename EdgeColorMap>
  void undirected_depth_first_visit
    (const IncidenceGraph& g,
     typename graph_traits<IncidenceGraph>::vertex_descriptor u, 
     DFSVisitor vis, VertexColorMap vertex_color, EdgeColorMap edge_color)
  {
    detail::undir_dfv_impl(g, u, vis, vertex_color, edge_color);
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS
} // namespace boost

#endif
