/* make_random_paths.cpp source file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

/*
 * Defines the Boost Unit Test Framework.
 */
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/included/unit_test_framework.hpp>

/*
 * Defines the std::list class template and its iterators.
 */
#include <list>

/*
 * Defines the std::map class template.
 */
#include <map>

/*
 * Defines the boost::mt19937 class, to be used as a random-number-generating
 * engine.
 */
#include <boost/random/mersenne_twister.hpp>

/*
 * Defines the boost::uniform_int class template, to be used as a random-number
 * distribution.
 */
#include <boost/random/uniform_int.hpp>

/*
 * Defines the boost::variate_generator class template, to be used as the
 * front-end random-number generator.
 */
#include <boost/random/variate_generator.hpp>

/*
 * Defines the boost::random_number_generator class template, to be used as the
 * front-end random-index generator.
 */
#include <boost/random/random_number_generator.hpp>

/*
 * Defines the boost::property_map and boost::associative_property_map class
 * templates and the boost::get and boost::put function templates.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::is_adjacent function template.
 */
#include <boost/graph/graph_utility.hpp>

/*
 * Defines the boost::graph_traits class template.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the vertex and edge property tags.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::directedS, boost::undirectedS, and boost::bidirectionalS
 * selector tags.
 */
#include <boost/graph/graph_selectors.hpp>

/*
 * Defines the boost::adjacency_list class template and its associated
 * non-member function templates.
 */
#include <boost/graph/adjacency_list.hpp>

/*
 * Defines the boost::adjacency_matrix class template and its associated
 * non-member function templates.
 */
#include <boost/graph/adjacency_matrix.hpp>

/*
 * Defines the boost::dijkstra_random_paths function template.
 */
#include <boost/graph/dijkstra_random_paths.hpp>

/*
 * Defines the boost::ddnw_random_paths function template.
 */
#include <boost/graph/ddnw_random_paths.hpp>

/*
 * Defines the boost::loop_erased_random_paths function template.
 */
#include <boost/graph/loop_erased_random_paths.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost;

template <typename Graph>
bool contains_no_edges(Graph& g)
{
    typename graph_traits<Graph>::edge_iterator ei, eend;

    tie(ei, eend) = edges(g);

    return ei == eend;
}

template <typename Graph>
void init_graph(Graph& g)
{
    add_edge(vertex(0, g), vertex(7, g), g);
    add_edge(vertex(1, g), vertex(2, g), g);
    add_edge(vertex(2, g), vertex(10, g), g);
    add_edge(vertex(2, g), vertex(5, g), g);
    add_edge(vertex(3, g), vertex(10, g), g);
    add_edge(vertex(3, g), vertex(0, g), g);
    add_edge(vertex(4, g), vertex(5, g), g);
    add_edge(vertex(4, g), vertex(0, g), g);
    add_edge(vertex(5, g), vertex(14, g), g);
    add_edge(vertex(6, g), vertex(3, g), g);
    add_edge(vertex(7, g), vertex(17, g), g);
    add_edge(vertex(7, g), vertex(11, g), g);
    add_edge(vertex(8, g), vertex(17, g), g);
    add_edge(vertex(8, g), vertex(1, g), g);
    add_edge(vertex(9, g), vertex(11, g), g);
    add_edge(vertex(9, g), vertex(1, g), g);
    add_edge(vertex(10, g), vertex(19, g), g);
    add_edge(vertex(10, g), vertex(15, g), g);
    add_edge(vertex(10, g), vertex(8, g), g);
    add_edge(vertex(11, g), vertex(19, g), g);
    add_edge(vertex(11, g), vertex(15, g), g);
    add_edge(vertex(11, g), vertex(4, g), g);
    add_edge(vertex(12, g), vertex(19, g), g);
    add_edge(vertex(12, g), vertex(8, g), g);
    add_edge(vertex(12, g), vertex(4, g), g);
    add_edge(vertex(13, g), vertex(15, g), g);
    add_edge(vertex(13, g), vertex(8, g), g);
    add_edge(vertex(13, g), vertex(4, g), g);
    add_edge(vertex(14, g), vertex(22, g), g);
    add_edge(vertex(14, g), vertex(12, g), g);
    add_edge(vertex(15, g), vertex(22, g), g);
    add_edge(vertex(15, g), vertex(6, g), g);
    add_edge(vertex(16, g), vertex(12, g), g);
    add_edge(vertex(16, g), vertex(6, g), g);
    add_edge(vertex(17, g), vertex(20, g), g);
    add_edge(vertex(18, g), vertex(9, g), g);
    add_edge(vertex(19, g), vertex(23, g), g);
    add_edge(vertex(19, g), vertex(18, g), g);
    add_edge(vertex(20, g), vertex(23, g), g);
    add_edge(vertex(20, g), vertex(13, g), g);
    add_edge(vertex(21, g), vertex(18, g), g);
    add_edge(vertex(21, g), vertex(13, g), g);
    add_edge(vertex(22, g), vertex(21, g), g);
    add_edge(vertex(23, g), vertex(16, g), g);
}

template <typename Graph, typename RNGEngine>
void dijkstra_random_paths_test()
{
    typedef typename graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef typename graph_traits<Graph>::vertex_iterator
            VertexIterator;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename property_map<Graph,vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<Vertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef uniform_int<unsigned int>
            RNGDistribution;
    typedef variate_generator<RNGEngine&,RNGDistribution>
            RandomNumberGenerator;

    Graph                 g(24);
    VertexIterator        source_itr, source_end, target_itr, target_end;
    Vertex                v;
    VertexMap             v_map;
    PredecessorMap        pred_map(v_map);
    VertexIndexMap        index_map = get(vertex_index_t(), g);
    Path                  path;
    PathIterator          pi, pi_next;
    RNGEngine             rng_engine;
    RNGDistribution       rng_distribution(0, 16384);
    RandomNumberGenerator random_weight(rng_engine, rng_distribution);

    init_graph(g);

    for (tie(source_itr, source_end) = vertices(g);
         source_itr != source_end;
         ++source_itr)
    {
        dijkstra_random_paths(
            g, *source_itr, random_weight,
            predecessor_map(pred_map));

        for (tie(target_itr, target_end) = vertices(g);
             target_itr != target_end;
             ++target_itr)
        {
            if (source_itr != target_itr)
            {
                for (v = *target_itr;
                     v != get(pred_map, v);
                     v = get(pred_map, v))
                {
                    path.push_front(v);
                }

                path.push_front(v);

                BOOST_CHECK_MESSAGE(
                    v == *source_itr,
                    "No path from " << get(index_map, *source_itr) << " to "
                                    << get(index_map, *target_itr));

                pi_next = path.begin();

                for (pi = pi_next; ++pi_next != path.end(); pi = pi_next)
                {
                    BOOST_CHECK_MESSAGE(
                        (is_adjacent(g, *pi, *pi_next)),
                        "Path edge " << get(index_map, *pi) << "-"
                                     << get(index_map, *pi_next)
                                     << " not part of graph.");
                }

                path.clear();
            }
        }
    }
}

template <typename InputGraph, typename UtilGraph, typename RNGEngine>
void ddnw_random_paths_test()
{
    typedef typename graph_traits<InputGraph>::vertex_descriptor
            InputVertex;
    typedef typename graph_traits<InputGraph>::vertex_iterator
            InputVertexIterator;
    typedef std::map<InputVertex,InputVertex>
            OutputVertexMap;
    typedef associative_property_map<OutputVertexMap>
            OutputPredecessorMap;
    typedef typename property_map<InputGraph,vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<InputVertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef uniform_int<unsigned int>
            RNGDistribution;
    typedef variate_generator<RNGEngine&,RNGDistribution>
            RandomNumberGenerator;

    InputGraph            g(24);
    UtilGraph             dag(24);
    InputVertexIterator   source_itr, source_end, target_itr, target_end;
    InputVertex           v;
    OutputVertexMap       out_v_map;
    OutputPredecessorMap  out_pred_map(out_v_map);
    VertexIndexMap        index_map = get(vertex_index_t(), g);
    Path                  path;
    PathIterator          pi, pi_next;
    RNGEngine             rng_engine;
    RNGDistribution       rng_distribution(0, 16384);
    RandomNumberGenerator random_weight(rng_engine, rng_distribution);

    init_graph(g);

    for (tie(source_itr, source_end) = vertices(g);
         source_itr != source_end;
         ++source_itr)
    {
        BOOST_CHECK_MESSAGE(
            num_vertices(dag) == num_vertices(g),
            "Utility graph doesn't have the same vertex count as input graph.");
        BOOST_CHECK_MESSAGE(
            contains_no_edges(dag), "Utility graph contains an edge.");

        try
        {
            ddnw_random_paths(
                g, *source_itr, random_weight, dag,
                predecessor_map(out_pred_map),
                vertex_index_map(get(vertex_index_t(), dag)));
        }
        catch (not_a_dag&)
        {
            BOOST_ERROR("ddnw_random_paths did not make utility graph a dag.");
            continue;
        }

        for (tie(target_itr, target_end) = vertices(g);
             target_itr != target_end;
             ++target_itr)
        {
            if (source_itr != target_itr)
            {
                for (v = *target_itr;
                     v != get(out_pred_map, v);
                     v = get(out_pred_map, v))
                {
                    path.push_front(v);
                }

                path.push_front(v);

                BOOST_CHECK_MESSAGE(
                    v == *source_itr,
                    "No path from " << get(index_map, *source_itr) << " to "
                                    << get(index_map, *target_itr));

                pi_next = path.begin();

                for (pi = pi_next; ++pi_next != path.end(); pi = pi_next)
                {
                    BOOST_CHECK_MESSAGE(
                        (is_adjacent(g, *pi, *pi_next)),
                        "Path edge " << get(index_map, *pi) << "-"
                                     << get(index_map, *pi_next)
                                     << " not part of graph.");
                }

                path.clear();
            }
        }
    }
}

template <typename Graph, typename RNGEngine>
void loop_erased_random_paths_test()
{
    typedef typename graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef typename graph_traits<Graph>::vertex_iterator
            VertexIterator;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename property_map<Graph,vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<Vertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef random_number_generator<RNGEngine,unsigned int>
            RandomIndexGenerator;

    Graph                g(24);
    Graph                u_g(24);
    VertexIterator       source_itr, source_end, target_itr, target_end;
    Vertex               v;
    VertexMap            v_map;
    PredecessorMap       pred_map(v_map);
    VertexIndexMap       index_map = get(vertex_index_t(), g);
    Path                 path;
    PathIterator         pi, pi_next;
    RNGEngine            rng_engine;
    RandomIndexGenerator rig(rng_engine);

    init_graph(g);

    for (tie(source_itr, source_end) = vertices(g);
         source_itr != source_end;
         ++source_itr)
    {
        BOOST_CHECK_MESSAGE(
            num_vertices(u_g) == num_vertices(g),
            "Utility graph doesn't have the same vertex count as input graph.");
        BOOST_CHECK_MESSAGE(
            contains_no_edges(u_g), "Utility graph contains an edge.");
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
        while (!loop_erased_random_paths(g, *source_itr, rig, pred_map, u_g));
#else
        while (!loop_erased_random_paths(
            g, *source_itr, rig, u_g,
            predecessor_map(pred_map),
            vertex_index_map(get(vertex_index_t(), u_g))));
#endif

        for (tie(target_itr, target_end) = vertices(g);
             target_itr != target_end;
             ++target_itr)
        {
            if (source_itr != target_itr)
            {
                for (v = *target_itr;
                     v != get(pred_map, v);
                     v = get(pred_map, v))
                {
                    path.push_front(v);
                }

                path.push_front(v);

                BOOST_CHECK_MESSAGE(
                    v == *source_itr,
                    "No path from " << get(index_map, *source_itr) << " to "
                                    << get(index_map, *target_itr));

                pi_next = path.begin();

                for (pi = pi_next; ++pi_next != path.end(); pi = pi_next)
                {
                    BOOST_CHECK_MESSAGE(
                        (is_adjacent(g, *pi, *pi_next)),
                        "Path edge " << get(index_map, *pi) << "-"
                                     << get(index_map, *pi_next)
                                     << " not part of graph.");
                    }

                path.clear();
            }
        }
    }
}

template <typename UndirectedGraph, typename DirectedGraph, typename RNGEngine>
void test()
{
    BOOST_MESSAGE(
        "Testing dijkstra_random_paths function template on undirected graph.");
    dijkstra_random_paths_test<UndirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing ddnw_random_paths function template on undirected graph.");
    ddnw_random_paths_test<UndirectedGraph,DirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing loop_erased_random_paths function template on " <<
        "undirected graph.");
    loop_erased_random_paths_test<UndirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing dijkstra_random_paths function template on directed graph.");
    dijkstra_random_paths_test<DirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing ddnw_random_paths function template on directed graph.");
    ddnw_random_paths_test<DirectedGraph,DirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing loop_erased_random_paths function template on " <<
        "directed graph.");
    loop_erased_random_paths_test<DirectedGraph,RNGEngine>();
}

template <typename EdgeContainerSelector, typename RNGEngine>
void vec_adjacency_list_test()
{
    typedef adjacency_list<EdgeContainerSelector,vecS,undirectedS,
                           no_property,property<edge_weight_t,int>,no_property>
            UndirectedGraph;
    typedef adjacency_list<EdgeContainerSelector,vecS,directedS,
                           no_property,property<edge_weight_t,int>,no_property>
            DirectedGraph;
    typedef adjacency_list<EdgeContainerSelector,vecS,bidirectionalS,
                           no_property,property<edge_weight_t,int>,no_property>
            BidirectionalGraph;

    test<UndirectedGraph,DirectedGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing dijkstra_random_paths function template on bidigraph.");
    dijkstra_random_paths_test<BidirectionalGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing ddnw_random_paths function template on bidigraph.");
    ddnw_random_paths_test<BidirectionalGraph,BidirectionalGraph,RNGEngine>();

    BOOST_MESSAGE(
        "Testing loop_erased_random_paths function template on " <<
        "bidigraph.");
    loop_erased_random_paths_test<BidirectionalGraph,RNGEngine>();
}

template <typename RNGEngine>
void adjacency_matrix_test()
{
    typedef adjacency_matrix<
              undirectedS,no_property,property<edge_weight_t,int>,no_property>
            UndirectedGraph;
    typedef adjacency_matrix<
              directedS,no_property,property<edge_weight_t,int>,no_property>
            DirectedGraph;

    test<UndirectedGraph,DirectedGraph,RNGEngine>();
}

void vec_adjacency_list_vecS_test_case()
{
    BOOST_MESSAGE("vec_adjacency_list_test vecS");
    vec_adjacency_list_test<vecS,mt19937>();
}

void vec_adjacency_list_listS_test_case()
{
    BOOST_MESSAGE("vec_adjacency_list_test listS");
    vec_adjacency_list_test<listS,mt19937>();
}

void vec_adjacency_list_setS_test_case()
{
    BOOST_MESSAGE("vec_adjacency_list_test setS");
    vec_adjacency_list_test<setS,mt19937>();
}

void vec_adjacency_list_multisetS_test_case()
{
    BOOST_MESSAGE("vec_adjacency_list_test multisetS");
    vec_adjacency_list_test<multisetS,mt19937>();
}

void adjacency_matrix_test_case()
{
    BOOST_MESSAGE("adjacency_matrix_test");
    adjacency_matrix_test<mt19937>();
}

test_suite* init_unit_test_suite(int argc, char** argv)
{
    test_suite* test = BOOST_TEST_SUITE("make_random_paths unit test");

    test->add(BOOST_TEST_CASE(&vec_adjacency_list_vecS_test_case));
    test->add(BOOST_TEST_CASE(&vec_adjacency_list_listS_test_case));
    test->add(BOOST_TEST_CASE(&vec_adjacency_list_setS_test_case));
    test->add(BOOST_TEST_CASE(&vec_adjacency_list_multisetS_test_case));
    test->add(BOOST_TEST_CASE(&adjacency_matrix_test_case));

    return test;
}

