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
#ifndef BOOST_GRAPH_TOPOLOGICAL_SORT_HPP
#define BOOST_GRAPH_TOPOLOGICAL_SORT_HPP

#include <boost/config.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <boost/type_traits/remove_reference.hpp>
#include <boost/parameter/preprocessor.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/traits.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/exception.hpp>
#include <boost/throw_exception.hpp>

#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS) && \
    !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
#include <boost/core/enable_if.hpp>
#endif

namespace boost { 


  // Topological sort visitor
  //
  // This visitor merely writes the linear ordering into an
  // OutputIterator. The OutputIterator could be something like an
  // ostream_iterator, or it could be a back/front_insert_iterator.
  // Note that if it is a back_insert_iterator, the recorded order is
  // the reverse topological order. On the other hand, if it is a
  // front_insert_iterator, the recorded order is the topological
  // order.
  //
  template <typename OutputIterator>
  struct topo_sort_visitor : public dfs_visitor<>
  {
    topo_sort_visitor(OutputIterator _iter)
      : m_iter(_iter) { }
    
    template <typename Edge, typename Graph>
    void back_edge(const Edge&, Graph&) { BOOST_THROW_EXCEPTION(not_a_dag()); }
    
    template <typename Vertex, typename Graph> 
    void finish_vertex(const Vertex& u, Graph&) { *m_iter++ = u; }
    
    OutputIterator m_iter;
  };


  // Topological Sort
  //
  // The topological sort algorithm creates a linear ordering
  // of the vertices such that if edge (u,v) appears in the graph,
  // then u comes before v in the ordering. The graph must
  // be a directed acyclic graph (DAG). The implementation
  // consists mainly of a call to depth-first search.
  //
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (bool), topological_sort, ::boost::graph::keywords::tag,
    (required
      (graph, *(detail::argument_predicate<is_vertex_list_graph>))
    )
    (deduced
      (required
        (result, *(detail::argument_predicate<detail::is_iterator>))
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
      )
    )
  )
#else   // !defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
  BOOST_PARAMETER_FUNCTION(
    (
      boost::disable_if<
        detail::is_bgl_named_param_argument<
          Args,
          boost::graph::keywords::tag::vertex_index_map
        >,
        bool
      >
    ), topological_sort, ::boost::graph::keywords::tag,
    (required
      (graph, *)
      (result, *)
    )
    (optional
      (vertex_index_map
        ,*
        ,detail::vertex_or_dummy_property_map(graph, vertex_index)
      )
      (color_map
        ,*
        ,make_shared_array_property_map(
          num_vertices(graph),
          white_color,
          vertex_index_map
        )
      )
    )
  )
#endif  // BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS
  {
    depth_first_search(
      graph,
      topo_sort_visitor<
        typename boost::remove_const<
          typename boost::remove_reference<result_type>::type
        >::type
      >(result),
#if defined(BOOST_GRAPH_CONFIG_CAN_DEDUCE_UNNAMED_ARGUMENTS)
      vertex_index_map,
#endif
      color_map
    );
    return true;
  }
#endif  // BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS

  template <typename VertexListGraph, typename OutputIterator,
    typename P, typename T, typename R>
  void topological_sort(VertexListGraph& g, OutputIterator result,
                        const bgl_named_params<P, T, R>& params)
  {
#if defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
    typedef bgl_named_params<P, T, R> params_type;
    BOOST_GRAPH_DECLARE_CONVERTED_PARAMETERS(params_type, params)
    depth_first_search(
      g,
      boost::graph::keywords::_visitor = topo_sort_visitor<
        OutputIterator
      >(result),
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
      ]
    );
#else
    typedef topo_sort_visitor<OutputIterator> TopoVisitor;
    depth_first_search(g, params.visitor(TopoVisitor(result)));
#endif
  }

#if !defined(BOOST_GRAPH_CONFIG_CAN_NAME_ARGUMENTS)
  template <typename VertexListGraph, typename OutputIterator>
  void topological_sort(VertexListGraph& g, OutputIterator result)
  {
    topological_sort(g, result, 
                     bgl_named_params<int, buffer_param_t>(0)); // bogus
  }
#endif
} // namespace boost

#endif /*BOOST_GRAPH_TOPOLOGICAL_SORT_H*/
