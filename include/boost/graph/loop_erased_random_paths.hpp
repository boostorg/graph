/* boost/graph/loop_erased_random_paths.hpp header file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_GRAPH_LOOP_ERASED_RANDOM_PATHS_HPP
#define BOOST_GRAPH_LOOP_ERASED_RANDOM_PATHS_HPP

/*
 * Defines the std::vector class template.
 */
#include <vector>

/*
 * Defines the boost::function_requires function template and the basic concept
 * check templates.
 */
#include <boost/concept_check.hpp>

/*
 * Defines the boost::property_traits class template and the boost::get and
 * boost::put function templates.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::tie function template.
 */
#include <boost/utility.hpp>

/*
 * Defines the BGL concept check templates.
 */
#include <boost/graph/graph_concepts.hpp>

/*
 * Defines the boost::graph_traits class template and the directed category
 * tags.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the boost::vertex_index_t property tag.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::bgl_named_params class template and its helper function
 * templates.
 */
#include <boost/graph/named_function_params.hpp>

/*
 * The graph type and its associated non-member function templates must be
 * defined externally.
 */
//#include <boost/graph/adjacency_list.hpp>

/*
 * Defines the boost::uniform_int class template, to be used as a random-index
 * distribution.
 */
#include <boost/random/uniform_int.hpp>

/*
 * Defines the boost::variate_generator class template, to be used as the
 * front-end random index generator.
 */
#include <boost/random/variate_generator.hpp>

namespace boost {

  namespace detail {

    /*
     * The generic RandomSuccessor() function.
     */
    template <typename InputGraph, typename RandomIndexGenerator,
              typename InputIndexMap, typename UtilGraph, typename UtilIndexMap,
              typename UtilColorMap>
    typename graph_traits<InputGraph>::vertex_descriptor get_random_predecessor(
        typename graph_traits<InputGraph>::vertex_descriptor v,
        InputGraph& in_g, RandomIndexGenerator& rig, InputIndexMap in_index_map,
        UtilGraph& u_g, UtilIndexMap u_index_map, UtilColorMap u_color_map)
    {
        typedef typename property_traits<UtilColorMap>::value_type
                UtilColorValue;
        typedef color_traits<UtilColorValue>
                UtilColor;

        std::vector<typename graph_traits<UtilGraph>::vertex_descriptor> prev;
        typename graph_traits<UtilGraph>::vertex_descriptor u =
            vertex(get(in_index_map, v), u_g);
        typename graph_traits<UtilGraph>::adjacency_iterator ai, aend;

        for (tie(ai, aend) = adjacent_vertices(u, u_g); ai != aend; ++ai)
        {
            if (get(u_color_map, *ai) == UtilColor::white())
            {
                prev.push_back(*ai);
            }
        }

        if (prev.empty())
        {
            return v;
        }

        return vertex(get(u_index_map, prev[rig(prev.size())]), in_g);
    }
  }  // namespace detail

/*
 * The generic kitchen-sink variant.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename OutputPredecessorMap, typename InputIndexMap,
          typename InputColorMap, typename UtilGraph, typename UtilIndexMap,
          typename UtilColorMap>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map,
    InputIndexMap in_index_map, InputColorMap in_color_map, UtilGraph& u_g,
    UtilIndexMap u_index_map, UtilColorMap u_color_map)
{
    function_requires<
      VertexListGraphConcept<InputGraph> >();
    function_requires<
      AdjacencyGraphConcept<InputGraph> >();
    typedef graph_traits<InputGraph>
            InputGraphTraits;
    typedef typename InputGraphTraits::vertex_descriptor
            InputVertex;
    function_requires<
      ReadWritePropertyMapConcept<OutputPredecessorMap,InputVertex> >();
    function_requires<
      ReadablePropertyMapConcept<InputIndexMap,InputVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<InputColorMap,InputVertex> >();
    typedef typename property_traits<InputColorMap>::value_type
            InputColorValue;
    typedef color_traits<InputColorValue>
            InputColor;
    function_requires<
      VertexListGraphConcept<UtilGraph> >();
    function_requires<
      AdjacencyGraphConcept<UtilGraph> >();
    function_requires<
      EdgeMutableGraphConcept<UtilGraph> >();
    typedef graph_traits<UtilGraph>
            UtilGraphTraits;
    typedef typename UtilGraphTraits::vertex_descriptor
            UtilVertex;
    function_requires<
      ReadablePropertyMapConcept<UtilIndexMap,UtilVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<UtilColorMap,UtilVertex> >();
    typedef typename property_traits<UtilColorMap>::value_type
            UtilColorValue;
    typedef color_traits<UtilColorValue>
            UtilColor;

    typename InputGraphTraits::vertex_iterator vi, vend;
    typename InputGraphTraits::adjacency_iterator ai, aend;
    UtilVertex w;

    for (tie(vi, vend) = vertices(in_g); vi != vend; ++vi)
    {
        put(out_pred_map, *vi, *vi);
        put(in_color_map, *vi, InputColor::white());
        w = vertex(get(in_index_map, *vi), u_g);

        for (tie(ai, aend) = adjacent_vertices(*vi, in_g); ai != aend; ++ai)
        {
            add_edge(vertex(get(in_index_map, *ai), u_g), w, u_g);
        }
    }

    put(in_color_map, source, InputColor::black());

    InputVertex u, v;
    typename UtilGraphTraits::vertex_iterator ui, uend;
    bool is_tree = true;

    for (tie(vi, vend) = vertices(in_g); vi != vend; ++vi)
    {
        for (tie(ui, uend) = vertices(u_g); ui != uend; ++ui)
        {
            put(u_color_map, *ui, UtilColor::white());
        }

        for (v = *vi; get(in_color_map, v) == InputColor::white(); v = u)
        {
            put(u_color_map, get(in_index_map, v), UtilColor::black());
            u = detail::get_random_predecessor(
                v, in_g, rig, in_index_map, u_g, u_index_map, u_color_map);

            if (u == v)
            {
                is_tree = false;
            }

            put(out_pred_map, v, u);
            put(in_color_map, v, InputColor::black());
        }
    }

    for (tie(ui, uend) = vertices(u_g); ui != uend; ++ui)
    {
        clear_vertex(*ui, u_g);
    }

    return is_tree;
}

  namespace detail {

    template <typename InputGraph, typename RandomIndexGenerator,
              typename OutputPredecessorMap, typename InputIndexMap,
              typename InputColorMap, typename UtilGraph, typename UtilIndexMap,
              typename UtilColorMap, typename IP, typename IT, typename IR,
              typename UP, typename UT, typename UR>
    bool loop_erased_random_paths_dispatch(
        InputGraph& in_g,
        typename graph_traits<InputGraph>::vertex_descriptor source,
        RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map,
        InputIndexMap in_index_map, InputColorMap in_c_map, UtilGraph& u_g,
        UtilIndexMap u_index_map, UtilColorMap u_c_map,
        const bgl_named_params<IP, IT, IR>& in_params,
        const bgl_named_params<UP, UT, UR>& u_params)
    {
        // Default for color maps.
        std::vector<default_color_type> in_color_map(num_vertices(in_g));
        std::vector<default_color_type> u_color_map(num_vertices(u_g));

        return loop_erased_random_paths(
            in_g, source, rig, out_pred_map, in_index_map,
            choose_param(
                in_c_map,
                make_iterator_property_map(
                    in_color_map.begin(), in_index_map, white_color)),
            u_g, u_index_map,
            choose_param(
                u_c_map,
                make_iterator_property_map(
                    u_color_map.begin(), u_index_map, white_color)));
    }
  }  // namespace detail

/*
 * The named parameter variant.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename UtilGraph, typename IP, typename IT, typename IR,
          typename UP, typename UT, typename UR>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig, UtilGraph& u_g,
    const bgl_named_params<IP, IT, IR>& in_params,
    const bgl_named_params<UP, UT, UR>& u_params)
{
    return detail::loop_erased_random_paths_dispatch(
        in_g, source, rig,
        choose_pmap(
            get_param(in_params, vertex_predecessor_t()), in_g,
            vertex_predecessor_t()),
        choose_const_pmap(
            get_param(in_params, vertex_index_t()), in_g,
            vertex_index_t()),
        get_param(in_params, vertex_color_t()),
        u_g,
        choose_const_pmap(
            get_param(u_params, vertex_index_t()), u_g,
            vertex_index_t()),
        get_param(u_params, vertex_color_t()),
        in_params, u_params);
}

/*
 * The generic default variant.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename OutputPredecessorMap, typename UtilGraph>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map,
    UtilGraph& u_g)
{
    typedef typename property_map<InputGraph,vertex_index_t>::type
            InputIndexMap;
    typedef typename property_map<UtilGraph,vertex_index_t>::type
            UtilIndexMap;

    std::vector<default_color_type> in_color_map(num_vertices(in_g));
    std::vector<default_color_type> u_color_map(num_vertices(u_g));

    InputIndexMap in_index_map = get(vertex_index_t(), in_g);
    UtilIndexMap  u_index_map  = get(vertex_index_t(), u_g);

    return loop_erased_random_paths(
        in_g, source, rig, out_pred_map, in_index_map,
        make_iterator_property_map(
            in_color_map.begin(), in_index_map, white_color),
        u_g, u_index_map,
        make_iterator_property_map(
            u_color_map.begin(), u_index_map, white_color));
}

#ifdef BOOST_SANDBOX_LERP_SPECIAL
  namespace detail {

    /*
     * The undirected-graph RandomSuccessor() function.
     */
    template <typename InputGraph, typename RandomIndexGenerator,
              typename InputIndexMap, typename InputColorMap>
    typename graph_traits<InputGraph>::vertex_descriptor get_random_predecessor(
        typename graph_traits<InputGraph>::vertex_descriptor v,
        InputGraph& in_g, RandomIndexGenerator& rig, InputIndexMap in_index_map,
        InputColorMap in_color_map, undirected_tag)
    {
        typedef typename property_traits<InputColorMap>::value_type
                InputColorValue;
        typedef color_traits<InputColorValue>
                InputColor;

        std::vector<typename graph_traits<InputGraph>::vertex_descriptor> prev;
        typename graph_traits<InputGraph>::adjacency_iterator ai, aend;

        /*
         * In an undirected graph, there is no difference between the in-edges
         * and the out-edges of a vertex.
         */
        for (tie(ai, aend) = adjacent_vertices(v, in_g); ai != aend; ++ai)
        {
            if (get(in_color_map, *ai) == InputColor::white())
            {
                prev.push_back(*ai);
            }
        }

        if (prev.empty())
        {
            return v;
        }

        return prev[rig(prev.size())];
    }

    /*
     * The bidirectional-graph RandomSuccessor() function.
     */
    template <typename InputGraph, typename RandomIndexGenerator,
              typename InputIndexMap, typename InputColorMap>
    typename graph_traits<InputGraph>::vertex_descriptor get_random_predecessor(
        typename graph_traits<InputGraph>::vertex_descriptor v,
        InputGraph& in_g, RandomIndexGenerator& rig, InputIndexMap in_index_map,
        InputColorMap in_color_map, bidirectional_tag)
    {
        typedef typename property_traits<InputColorMap>::value_type
                InputColorValue;
        typedef color_traits<InputColorValue>
                InputColor;

        std::vector<typename graph_traits<InputGraph>::vertex_descriptor> prev;
        typename graph_traits<InputGraph>::vertex_descriptor u;
        typename graph_traits<InputGraph>::in_edge_iterator ii, iend;

        /*
         * The in_edges free function is not implemented for directed graphs
         * which are not bidirectional, hence the need for a utility graph.
         */
        for (tie(ii, iend) = in_edges(v, in_g); ii != iend; ++ii)
        {
            u = source(*ii, in_g);

            if (get(in_color_map, u) == InputColor::white())
            {
                prev.push_back(u);
            }
        }

        if (prev.empty())
        {
            return v;
        }

        return prev[rig(prev.size())];
    }
  }  // namespace detail

/*
 * This specialized kitchen-sink variant can only be used if the graph is either
 * undirected or bidirectional.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename OutputPredecessorMap, typename InputIndexMap,
          typename InputColorMap>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map,
    InputIndexMap in_index_map, InputColorMap in_color_map)
{
    function_requires<
      VertexListGraphConcept<InputGraph> >();
    function_requires<
      AdjacencyGraphConcept<InputGraph> >();
    typedef graph_traits<InputGraph>
            InputGraphTraits;
    typedef typename InputGraphTraits::vertex_descriptor
            InputVertex;
    function_requires<
      ReadWritePropertyMapConcept<OutputPredecessorMap,InputVertex> >();
    function_requires<
      ReadablePropertyMapConcept<InputIndexMap,InputVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<InputColorMap,InputVertex> >();
    typedef typename property_traits<InputColorMap>::value_type
            InputColorValue;
    typedef color_traits<InputColorValue>
            InputColor;

    typename InputGraphTraits::vertex_iterator ui, uend, vi, vend;

    for (tie(vi, vend) = vertices(in_g); vi != vend; ++vi)
    {
        put(out_pred_map, *vi, *vi);
        put(in_color_map, *vi, InputColor::white());
    }

    put(in_color_map, source, InputColor::black());

    InputVertex u, v;
    bool is_tree = true;

    for (tie(vi, vend) = vertices(in_g); vi != vend; ++vi)
    {
        for (v = *vi; get(in_color_map, v) == InputColor::white(); v = u)
        {
            u = detail::get_random_predecessor(
                v, in_g, rig, in_index_map, in_color_map,
                InputGraphTraits::directed_category());

            if (u == v)
            {
                is_tree = false;
            }

            put(out_pred_map, v, u);
            put(in_color_map, v, InputColor::black());
        }
    }

if (!is_tree) std::cout << "  non-tree" << std::endl;
    return is_tree;
}

  namespace detail {

    template <typename InputGraph, typename RandomIndexGenerator,
              typename OutputPredecessorMap, typename InputIndexMap,
              typename InputColorMap, typename IP, typename IT, typename IR>
    bool loop_erased_random_paths_dispatch(
        InputGraph& in_g,
        typename graph_traits<InputGraph>::vertex_descriptor source,
        RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map,
        InputIndexMap in_index_map, InputColorMap in_c_map,
        const bgl_named_params<IP, IT, IR>& in_params)
    {
        // Default for color maps.
        std::vector<default_color_type> in_color_map(num_vertices(in_g));

        return loop_erased_random_paths(
            in_g, source, rig, out_pred_map, in_index_map,
            choose_param(
                in_c_map,
                make_iterator_property_map(
                    in_color_map.begin(), in_index_map, white_color)));
    }
  }  // namespace detail

/*
 * This specialized named parameter variant can only be used if the graph is
 * either undirected or bidirectional.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename IP, typename IT, typename IR>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig,
    const bgl_named_params<IP, IT, IR>& in_params)
{
    return detail::loop_erased_random_paths_dispatch(
        in_g, source, rig,
        choose_pmap(
            get_param(in_params, vertex_predecessor_t()), in_g,
            vertex_predecessor_t()),
        choose_const_pmap(
            get_param(in_params, vertex_index_t()), in_g,
            vertex_index_t()),
        get_param(in_params, vertex_color_t()));
}

/*
 * This specialized default variant can only be used if the graph is either
 * undirected or bidirectional.
 */
template <typename InputGraph, typename RandomIndexGenerator,
          typename OutputPredecessorMap>
bool loop_erased_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomIndexGenerator& rig, OutputPredecessorMap out_pred_map)
{
    typename property_map<InputGraph,vertex_index_t>::type
        in_index_map = get(vertex_index_t(), in_g);
    std::vector<default_color_type> in_color_map(num_vertices(in_g));

    return loop_erased_random_paths(
        in_g, source, rig, out_pred_map, in_index_map,
        make_iterator_property_map(
            in_color_map.begin(), in_index_map, white_color));
}
#endif // BOOST_SANDBOX_LERP_SPECIAL
}  // namespace boost

#endif /* BOOST_GRAPH_LOOP_ERASED_RANDOM_PATHS_HPP */

