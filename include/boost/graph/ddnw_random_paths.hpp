/* boost/graph/ddnw_random_paths.hpp header file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */
#ifndef BOOST_GRAPH_DDNW_RANDOM_PATHS_HPP
#define BOOST_GRAPH_DDNW_RANDOM_PATHS_HPP

/*
 * Defines the std::vector class template.
 */
#include <vector>

/*
 * Defines the std::map class template.
 */
#include <map>

/*
 * Defines the std::numeric_limits class template.
 */
#include <boost/limits.hpp>

/*
 * Defines the boost::function_requires function template and the basic concept
 * check templates.
 */
#include <boost/concept_check.hpp>

/*
 * Defines the boost::property_traits and boost::associative_property_map class
 * templates and the boost::get and boost::put function templates.
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
 * Defines the boost::graph_traits class template and the boost::directed_tag
 * property tag.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the vertex and edge property tags.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::null_visitor class.
 */
#include <boost/graph/visitors.hpp>

/*
 * Defines the boost::dag_shortest_paths and boost::make_dijkstra_visitor
 * function templates.
 */
#include <boost/graph/dag_shortest_paths.hpp>

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
 * Defines the boost::dijkstra_random_paths function template.
 */
#include <boost/graph/dijkstra_random_paths.hpp>

namespace boost {

  namespace detail {

    /*
     * This visitor class copies edges that were either relaxed or not relaxed
     * as a result dijkstra_shortest_paths (i.e. tree edges and edges
     * with gray targets, or forward and cross edges) from the input graph to
     * the utility graph, but negates the corresponding weights.
     */
    template <typename UtilGraph, typename EventFilter, typename InputIndexMap,
              typename NegateFunctor>
    class mrp_edge_copier :
        public base_visitor<
            mrp_edge_copier<UtilGraph,EventFilter,InputIndexMap,
                            NegateFunctor> >
    {
     public:
        typedef EventFilter event_filter;

     private:
        UtilGraph&    m_dag;
        InputIndexMap m_index_map;
        NegateFunctor m_negate;

     public:
        mrp_edge_copier(UtilGraph& dag, InputIndexMap index_map,
                        NegateFunctor negate = NegateFunctor()) :
            m_dag(dag), m_index_map(index_map), m_negate(negate)
        {
        }

        ~mrp_edge_copier()
        {
        }

        template <typename Graph>
        void operator()(
            typename graph_traits<Graph>::edge_descriptor e, Graph& g)
        {
            typename graph_traits<UtilGraph>::edge_descriptor edge;
            bool created_edge;

            tie(edge, created_edge) =
                add_edge(
                    vertex(get(m_index_map, source(e, g)), m_dag),
                    vertex(get(m_index_map, target(e, g)), m_dag),
                    m_dag);

            if (created_edge)
            {
                put(get(edge_weight_t(), m_dag), edge,
                    m_negate(get(get(edge_weight_t(), g), e)));
            }
        }
    };
  }  // namespace detail

/*
 * The kitchen-sink variant.
 */
template <typename InputGraph, typename RandomNumberGenerator,
          typename OutputPredecessorMap, typename InputDistanceMap,
          typename InputIndexMap, typename InputColorMap,
          typename CompareFunctor, typename CombineFunctor,
          typename DistanceInfinity, typename DistanceZero,
          typename InputEventVisitorList, typename UtilGraph,
          typename UtilPredecessorMap, typename UtilDistanceMap,
          typename UtilIndexMap, typename UtilColorMap,
          typename UtilEventVisitorList>
void ddnw_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight, OutputPredecessorMap out_pred_map,
    InputDistanceMap in_dist_map, InputIndexMap in_index_map,
    InputColorMap in_color_map, CompareFunctor compare, CombineFunctor combine,
    DistanceInfinity infinity, DistanceZero zero, InputEventVisitorList in_vis,
    UtilGraph& dag, UtilPredecessorMap u_pred_map, UtilDistanceMap u_dist_map,
    UtilIndexMap u_index_map, UtilColorMap u_color_map,
    UtilEventVisitorList dag_vis)
{
    function_requires<
      VertexAndEdgeListGraphConcept<InputGraph> >();
    typedef typename graph_traits<InputGraph>::vertex_descriptor
            InputVertex;
    function_requires<
      ReadWritePropertyMapConcept<OutputPredecessorMap,InputVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<InputDistanceMap,InputVertex> >();
    typedef typename property_map<InputGraph,edge_weight_t>::type
            InputWeightMap;
    typedef typename property_traits<InputWeightMap>::value_type
            InputWeight;
    typedef typename property_traits<InputDistanceMap>::value_type
            InputDistance;
    function_requires<
      ConvertibleConcept<InputWeight,InputDistance> >();
    function_requires<
      ConvertibleConcept<DistanceInfinity,InputDistance> >();
    function_requires<
      ConvertibleConcept<DistanceZero,InputDistance> >();
    function_requires<
      ReadablePropertyMapConcept<InputIndexMap,InputVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<InputColorMap,InputVertex> >();
    function_requires<
      VertexAndEdgeListGraphConcept<UtilGraph> >();
    function_requires<
      EdgeMutableGraphConcept<UtilGraph> >();
    typedef typename graph_traits<UtilGraph>::directed_category
            DirectedCategory;
    function_requires<
      ConvertibleConcept<DirectedCategory,directed_tag> >();
    typedef typename graph_traits<UtilGraph>::vertex_descriptor
            UtilVertex;
    function_requires<
      ReadWritePropertyMapConcept<UtilPredecessorMap,UtilVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<UtilDistanceMap,UtilVertex> >();
    typedef typename property_map<UtilGraph,edge_weight_t>::type
            UtilWeightMap;
    typedef typename property_traits<UtilWeightMap>::value_type
            UtilWeight;
    typedef typename property_traits<UtilDistanceMap>::value_type
            UtilDistance;
    function_requires<
      ConvertibleConcept<UtilWeight,UtilDistance> >();
    function_requires<
      ConvertibleConcept<DistanceInfinity,UtilDistance> >();
    function_requires<
      ConvertibleConcept<DistanceZero,UtilDistance> >();
    function_requires<
      ReadablePropertyMapConcept<UtilIndexMap,UtilVertex> >();
    function_requires<
      ReadWritePropertyMapConcept<UtilColorMap,UtilVertex> >();
    typedef typename property_traits<InputIndexMap>::value_type
            IndexValue;
    typedef detail::mrp_edge_copier<UtilGraph,on_edge_relaxed,
                                    InputIndexMap,std::negate<UtilWeight> >
            RelaxedEdgeVisitor;
    typedef detail::mrp_edge_copier<UtilGraph,on_edge_not_relaxed,
                                    InputIndexMap,std::negate<UtilWeight> >
            NonRelaxedEdgeVisitor;
    typedef dijkstra_visitor<
              std::pair<RelaxedEdgeVisitor,
                std::pair<NonRelaxedEdgeVisitor,InputEventVisitorList> > >
            DVisitor;

    /*
     * For the utility graph, make copies of only the forward and cross edges
     * with respect to a random traversal from the source vertex.
     */
    RelaxedEdgeVisitor re_vis(dag, in_index_map);
    NonRelaxedEdgeVisitor nre_vis(dag, in_index_map);
    DVisitor d_vis(std::make_pair(re_vis, std::make_pair(nre_vis, in_vis)));

    dijkstra_random_paths(
        in_g, source, random_weight, out_pred_map, in_dist_map, in_index_map,
        in_color_map, compare, combine, infinity, zero, d_vis);

    /*
     * The utility graph should now be a directed acyclic graph.
     * Run the longest simple path algorithm on it.
     */
    dag_shortest_paths(
        dag, source, u_dist_map, get(edge_weight_t(), dag),
        u_color_map, u_pred_map, make_dijkstra_visitor(dag_vis),
        compare, combine, infinity, zero);

    /*
     * The resulting paths will be stored in the utility predecessor map.
     * Copy its contents into the output predecessor map.
     *
     * TODO:
     * 1. Don't execute this code if in_index_map and u_index_map refer to
     *    the same object.
     */
    typename graph_traits<InputGraph>::vertex_descriptor in_source, in_target;
    typename graph_traits<UtilGraph>::vertex_iterator    u_vi, u_vend;

    for (tie(u_vi, u_vend) = vertices(dag); u_vi != u_vend; ++u_vi)
    {
        in_source = vertex(get(u_index_map, *u_vi), in_g);
        in_target = vertex(get(u_index_map, get(u_pred_map, *u_vi)), in_g);
        put(out_pred_map, in_source, in_target);
        clear_vertex(*u_vi, dag);
    }
}

  namespace detail {

    template <typename InputGraph, typename RandomNumberGenerator,
              typename OutputPredecessorMap, typename InputDistanceMap,
              typename InputIndexMap, typename InputColorMap,
              typename UtilGraph, typename UtilDistanceMap,
              typename UtilIndexMap, typename UtilColorMap,
              typename IP, typename IT, typename IR,
              typename UP, typename UT, typename UR>
    void ddnw_random_paths_dispatch(
        InputGraph& in_g,
        typename graph_traits<InputGraph>::vertex_descriptor source,
        RandomNumberGenerator& random_weight,
        OutputPredecessorMap out_pred_map, InputDistanceMap in_d_map,
        InputIndexMap in_index_map, InputColorMap in_color_map, UtilGraph& dag,
        UtilDistanceMap u_d_map, UtilIndexMap u_index_map,
        UtilColorMap u_color_map, const bgl_named_params<IP, IT, IR> in_params,
        const bgl_named_params<UP, UT, UR>& u_params)
    {
        // Default for utility predecessor map.
        typedef typename property_traits<InputDistanceMap>::value_type
                Distance;
        typedef typename graph_traits<InputGraph>::vertex_descriptor
                UtilVertex;
        typedef std::map<UtilVertex, UtilVertex>
                UtilVertexMap;
        typedef associative_property_map<UtilVertexMap>
                UtilPredecessorMap;

        UtilVertexMap      u_v_map;
        UtilPredecessorMap u_pred_map(u_v_map);
        null_visitor       n_vis;

        ddnw_random_paths(
            in_g, source, random_weight, out_pred_map,
            in_d_map, in_index_map, in_color_map,
            choose_param(
                get_param(in_params, distance_compare_t()),
                std::less<Distance>()),
            choose_param(
                get_param(in_params, distance_combine_t()),
                closed_plus<Distance>()),
            choose_param(
                get_param(in_params, distance_inf_t()),
                std::numeric_limits<Distance>::max()),
            choose_param(
                get_param(in_params, distance_zero_t()),
                Distance()),
            choose_param(
                get_param(in_params, graph_visitor_t()),
                n_vis),
            dag,
            choose_param(
                get_param(u_params, vertex_predecessor_t()),
                u_pred_map),
            u_d_map, u_index_map, u_color_map,
            choose_param(
                get_param(u_params, graph_visitor_t()),
                n_vis));
    }

    template <typename InputGraph, typename RandomNumberGenerator,
              typename OutputPredecessorMap, typename InputDistanceMap,
              typename InputIndexMap, typename InputColorMap,
              typename UtilGraph, typename UtilDistanceMap,
              typename UtilIndexMap, typename UtilColorMap,
              typename IP, typename IT, typename IR,
              typename UP, typename UT, typename UR>
    void ddnw_random_paths_dispatch2(
        InputGraph& in_g,
        typename graph_traits<InputGraph>::vertex_descriptor source,
        RandomNumberGenerator& random_weight,
        OutputPredecessorMap out_pred_map, InputDistanceMap in_d_map,
        InputIndexMap in_index_map, InputColorMap in_color_map, UtilGraph& dag,
        UtilDistanceMap u_d_map, UtilIndexMap u_index_map,
        UtilColorMap u_color_map, const bgl_named_params<IP, IT, IR> in_params,
        const bgl_named_params<UP, UT, UR>& u_params)
    {
        // Defaults for distance maps.
        typedef typename property_map<InputGraph,
                                             edge_weight_t>::type
                WeightMap;
        typedef typename property_traits<WeightMap>::value_type
                Distance;

        const Distance zero = choose_param(
                                  get_param(in_params, distance_zero_t()),
                                  Distance());

        std::vector<Distance> in_dist_map(
            is_default_param(in_d_map) ? num_vertices(in_g) : 1);
        std::vector<Distance> u_dist_map(
            is_default_param(u_d_map) ? num_vertices(dag) : 1);

        ddnw_random_paths_dispatch(
            in_g, source, random_weight, out_pred_map,
            choose_param(
                in_d_map,
                make_iterator_property_map(
                    in_dist_map.begin(), in_index_map, zero)),
            in_index_map, in_color_map, dag,
            choose_param(
                u_d_map,
                make_iterator_property_map(
                    u_dist_map.begin(), u_index_map, zero)),
            u_index_map, u_color_map, in_params, u_params);
    }

    template <typename InputGraph, typename RandomNumberGenerator,
              typename OutputPredecessorMap, typename InputDistanceMap,
              typename InputIndexMap, typename InputColorMap,
              typename UtilGraph, typename UtilDistanceMap,
              typename UtilIndexMap, typename UtilColorMap,
              typename IP, typename IT, typename IR,
              typename UP, typename UT, typename UR>
    void ddnw_random_paths_dispatch3(
        InputGraph& in_g,
        typename graph_traits<InputGraph>::vertex_descriptor source,
        RandomNumberGenerator& random_weight,
        OutputPredecessorMap out_pred_map, InputDistanceMap in_d_map,
        InputIndexMap in_index_map, InputColorMap in_c_map, UtilGraph& dag,
        UtilDistanceMap u_d_map, UtilIndexMap u_index_map,
        UtilColorMap u_c_map, const bgl_named_params<IP, IT, IR> in_params,
        const bgl_named_params<UP, UT, UR>& u_params)
    {
        // Default for color maps.
        std::vector<default_color_type> in_color_map(num_vertices(in_g));
        std::vector<default_color_type> u_color_map(num_vertices(dag));

        ddnw_random_paths_dispatch2(
            in_g, source, random_weight, out_pred_map, in_d_map, in_index_map,
            choose_param(
                in_c_map,
                make_iterator_property_map(
                    in_color_map.begin(), in_index_map, white_color)),
            dag, u_d_map, u_index_map,
            choose_param(
                u_c_map,
                make_iterator_property_map(
                    u_color_map.begin(), u_index_map, white_color)),
            in_params, u_params);
    }
  }  // namespace detail

/*
 * The named parameter variant.  There are two named parameters because
 * there are two graph parameters.
 */
template <typename InputGraph, typename RandomNumberGenerator,
          typename UtilGraph, typename IP, typename IT, typename IR,
          typename UP, typename UT, typename UR>
void ddnw_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight, UtilGraph& dag,
    const bgl_named_params<IP, IT, IR> in_params,
    const bgl_named_params<UP, UT, UR>& u_params)
{
    detail::ddnw_random_paths_dispatch3(
        in_g, source, random_weight,
        choose_pmap(
            get_param(in_params, vertex_predecessor_t()), in_g,
            vertex_predecessor_t()),
        get_param(in_params, vertex_distance_t()),
        choose_const_pmap(
            get_param(in_params, vertex_index_t()), in_g,
            vertex_index_t()),
        get_param(in_params, vertex_color_t()),
        dag,
        get_param(u_params, vertex_distance_t()),
        choose_const_pmap(
            get_param(u_params, vertex_index_t()), dag,
            vertex_index_t()),
        get_param(u_params, vertex_color_t()),
        in_params, u_params);
}

/*
 * The default variant.
 */
template <typename InputGraph, typename RandomNumberGenerator,
          typename OutputPredecessorMap, typename UtilGraph>
void ddnw_random_paths(
    InputGraph& in_g,
    typename graph_traits<InputGraph>::vertex_descriptor source,
    RandomNumberGenerator& random_weight, OutputPredecessorMap out_pred_map,
    UtilGraph& dag)
{
    typedef typename property_map<InputGraph,vertex_index_t>::type
            InputIndexMap;
    typedef typename property_map<UtilGraph,vertex_index_t>::type
            UtilIndexMap;
    typedef typename property_map<InputGraph,edge_weight_t>::type
            WeightMap;
    typedef typename property_traits<WeightMap>::value_type
            Distance;
    typedef typename graph_traits<InputGraph>::vertex_descriptor
            UtilVertex;
    typedef std::map<UtilVertex,UtilVertex>
            UtilVertexMap;
    typedef associative_property_map<UtilVertexMap>
            UtilPredecessorMap;

    const Distance zero = Distance();
    const typename graph_traits<InputGraph>::vertices_size_type
        n = num_vertices(in_g);

    UtilVertexMap                   u_v_map;
    UtilPredecessorMap              u_pred_map(u_v_map);
    null_visitor                    n_vis;
    std::vector<default_color_type> in_color_map(n);
    std::vector<default_color_type> u_color_map(n);
    std::vector<Distance>           in_dist_map(n);
    std::vector<Distance>           u_dist_map(n);

    InputIndexMap in_index_map = get(vertex_index_t(), in_g);
    UtilIndexMap  u_index_map  = get(vertex_index_t(), dag);

    ddnw_random_paths(
        in_g, source, random_weight, out_pred_map,
        make_iterator_property_map(in_dist_map.begin(), in_index_map, zero),
        in_index_map, std::less<Distance>(), closed_plus<Distance>(),
        std::numeric_limits<Distance>::max(), zero, n_vis, dag, u_pred_map,
        make_iterator_property_map(u_dist_map.begin(), u_index_map, zero),
        u_index_map,
        make_iterator_property_map(color.begin(), u_index_map, white_color),
        n_vis);
}
}  // namespace boost

#endif /* BOOST_GRAPH_DDNW_RANDOM_PATHS_HPP */

