/* boost/graph/dijkstra_random_paths.hpp header file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_GRAPH_DIJKSTRA_RANDOM_PATHS
#define BOOST_GRAPH_DIJKSTRA_RANDOM_PATHS

/*
 * Defines the std::vector class template.
 */
#include <vector>

/*
 * Defines the std::numeric_limits class template.
 */
#include <boost/limits.hpp>

/*
 * Defines the boost::property_traits class template and the boost::get and
 * boost::put function templates.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::graph_traits class template and the boost::is_directed
 * function template.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the vertex and edge property tags.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::randomize_property function template.
 */
#include <boost/graph/random.hpp>

/*
 * Defines the boost::dijkstra_shortest_paths and boost::make_dijkstra_visitor
 * function templates.
 */
#include <boost/graph/dijkstra_shortest_paths.hpp>

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

namespace boost {

/*
 * The kitchen-sink variant.
 */
template <typename Graph, typename RandomNumberGenerator,
          typename PredecessorMap, typename DistanceMap,
          typename VertexIndexMap, typename VertexColorMap,
          typename CompareFunctor, typename CombineFunctor,
          typename DistanceInfinity, typename DistanceZero,
          typename EventVisitorList>
void dijkstra_random_paths(
    Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight, PredecessorMap pred_map,
    DistanceMap dist_map, VertexIndexMap index_map, VertexColorMap color_map,
    CompareFunctor compare, CombineFunctor combine, DistanceInfinity infinity,
    DistanceZero zero, EventVisitorList vis)
{
    randomize_property<edge_weight_t,Graph,RandomNumberGenerator>(
        g, random_weight);
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
    dijkstra_shortest_paths(
        g, source, pred_map, dist_map, get(edge_weight_t(), g),
        index_map, compare, combine, infinity, zero, vis);
#else
    dijkstra_shortest_paths(
        g, source,
        weight_map(get(edge_weight_t(), g)).
            vertex_index_map(index_map).predecessor_map(pred_map).
            distance_map(dist_map).distance_compare(compare).
            distance_combine(combine).distance_inf(infinity).
            distance_zero(zero).color_map(color_map).visitor(vis));
#endif
}

  namespace detail {

    template <typename Graph, typename RandomNumberGenerator,
              typename PredecessorMap, typename DistanceMap,
              typename VertexIndexMap, typename VertexColorMap,
              typename Param, typename Tag, typename Rest>
    void dijkstra_random_paths_dispatch(
        Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
        RandomNumberGenerator& random_weight, PredecessorMap pred_map,
        DistanceMap d_map, VertexIndexMap index_map, VertexColorMap color_map,
        const bgl_named_params<Param, Tag, Rest>& params)
    {
        // Default for distance map.
        typedef typename property_map<Graph,edge_weight_t>::type
                WeightMap;
        typedef typename property_traits<WeightMap>::value_type
                Distance;

        const Distance zero = choose_param(
                                  get_param(params, distance_zero_t()),
                                  Distance());

        std::vector<Distance> distance_map(
            is_default_param(d_map) ? num_vertices(g) : 1);

        dijkstra_random_paths(
            g, source, random_weight, pred_map,
            choose_param(
                d_map,
                make_iterator_property_map(
                    distance_map.begin(), index_map, zero)),
            index_map, color_map,
            choose_param(
                get_param(params, distance_compare_t()),
                std::less<Distance>()),
            choose_param(
                get_param(params, distance_combine_t()),
                closed_plus<Distance>()),
            choose_param(
                get_param(params, distance_inf_t()),
                std::numeric_limits<Distance>::max()),
            zero,
            choose_param(
                get_param(params, graph_visitor_t()),
                default_dijkstra_visitor()));
    }

    template <typename Graph, typename RandomNumberGenerator,
              typename PredecessorMap, typename DistanceMap,
              typename VertexIndexMap, typename VertexColorMap,
              typename Param, typename Tag, typename Rest>
    void dijkstra_random_paths_dispatch2(
        Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
        RandomNumberGenerator& random_weight, PredecessorMap pred_map,
        DistanceMap d_map, VertexIndexMap index_map, VertexColorMap color_map,
        const bgl_named_params<Param, Tag, Rest>& params)
    {
        // Default for color map.
        std::vector<default_color_type> color(num_vertices(g));

        dijkstra_random_paths_dispatch(
            g, source, random_weight, pred_map, d_map, index_map,
            choose_param(
                color_map,
                make_iterator_property_map(
                    color.begin(), index_map, white_color)),
            params);
    }
  }  // namespace detail

/*
 * The named parameter variant.
 */
template <typename Graph, typename RandomNumberGenerator,
          typename Param, typename Tag, typename Rest>
void dijkstra_random_paths(
    Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight,
    const bgl_named_params<Param, Tag, Rest>& params)
{
    detail::dijkstra_random_paths_dispatch2(
        g, source, random_weight,
        choose_pmap(
            get_param(params, vertex_predecessor_t()), g,
            vertex_predecessor_t()),
        get_param(params, vertex_distance_t()),
        choose_pmap(
            get_param(params, vertex_index_t()), g,
            vertex_index_t()),
        get_param(params, vertex_color_t()),
        params);
}

/*
 * The default variant.
 */
template <typename Graph, typename RandomNumberGenerator,
          typename PredecessorMap>
void dijkstra_random_paths(
    Graph& g, typename graph_traits<Graph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight, PredecessorMap pred_map)
{
    typedef typename property_map<Graph,edge_weight_t>::type
            WeightMap;
    typedef typename property_traits<WeightMap>::value_type
            Distance;

    const Distance zero = Distance();

    typename property_map<Graph,vertex_index_t>::type
        index_map = get(vertex_index_t(), g);
    std::vector<Distance> distance_map(num_vertices(g));
    std::vector<default_color_type> color(num_vertices(g));

    dijkstra_random_paths(
        g, source, random_weight, pred_map,
        make_iterator_property_map(distance_map.begin(), index_map, zero),
        index_map,
        make_iterator_property_map(color.begin(), index_map, white_color),
        std::less<Distance>(), closed_plus<Distance>(),
        std::numeric_limits<Distance>::max(), zero, default_dijkstra_visitor());
}
}  // namespace boost

#endif /* BOOST_GRAPH_DIJKSTRA_RANDOM_PATHS */

