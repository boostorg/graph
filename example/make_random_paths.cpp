/* make_random_paths.cpp source file
 *
 * Copyright 2004 Cromwell D. Enage.  Covered by the Boost Software License,
 * Version 1.0.  (See accompanying file LICENSE_1_0.txt or copy at
 * http://www.boost.org/LICENSE_1_0.txt)
 */

/*
 * Defines the std::ios class and std::cout, its global output instance.
 */
#include <iostream>

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
 * Defines the boost::graph_traits class template.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the vertex and edge property tags.
 */
#include <boost/graph/properties.hpp>

/*
 * Defines the boost::directedS and boost::undirectedS selector tags.
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

template <typename Graph>
void init_graph(Graph& g)
{
    boost::add_edge(boost::vertex(0, g), boost::vertex(7, g), g);
    boost::add_edge(boost::vertex(1, g), boost::vertex(2, g), g);
    boost::add_edge(boost::vertex(2, g), boost::vertex(10, g), g);
    boost::add_edge(boost::vertex(2, g), boost::vertex(5, g), g);
    boost::add_edge(boost::vertex(3, g), boost::vertex(10, g), g);
    boost::add_edge(boost::vertex(3, g), boost::vertex(0, g), g);
    boost::add_edge(boost::vertex(4, g), boost::vertex(5, g), g);
    boost::add_edge(boost::vertex(4, g), boost::vertex(0, g), g);
    boost::add_edge(boost::vertex(5, g), boost::vertex(14, g), g);
    boost::add_edge(boost::vertex(6, g), boost::vertex(3, g), g);
    boost::add_edge(boost::vertex(7, g), boost::vertex(17, g), g);
    boost::add_edge(boost::vertex(7, g), boost::vertex(11, g), g);
    boost::add_edge(boost::vertex(8, g), boost::vertex(17, g), g);
    boost::add_edge(boost::vertex(8, g), boost::vertex(1, g), g);
    boost::add_edge(boost::vertex(9, g), boost::vertex(11, g), g);
    boost::add_edge(boost::vertex(9, g), boost::vertex(1, g), g);
    boost::add_edge(boost::vertex(10, g), boost::vertex(19, g), g);
    boost::add_edge(boost::vertex(10, g), boost::vertex(15, g), g);
    boost::add_edge(boost::vertex(10, g), boost::vertex(8, g), g);
    boost::add_edge(boost::vertex(11, g), boost::vertex(19, g), g);
    boost::add_edge(boost::vertex(11, g), boost::vertex(15, g), g);
    boost::add_edge(boost::vertex(11, g), boost::vertex(4, g), g);
    boost::add_edge(boost::vertex(12, g), boost::vertex(19, g), g);
    boost::add_edge(boost::vertex(12, g), boost::vertex(8, g), g);
    boost::add_edge(boost::vertex(12, g), boost::vertex(4, g), g);
    boost::add_edge(boost::vertex(13, g), boost::vertex(15, g), g);
    boost::add_edge(boost::vertex(13, g), boost::vertex(8, g), g);
    boost::add_edge(boost::vertex(13, g), boost::vertex(4, g), g);
    boost::add_edge(boost::vertex(14, g), boost::vertex(22, g), g);
    boost::add_edge(boost::vertex(14, g), boost::vertex(12, g), g);
    boost::add_edge(boost::vertex(15, g), boost::vertex(22, g), g);
    boost::add_edge(boost::vertex(15, g), boost::vertex(6, g), g);
    boost::add_edge(boost::vertex(16, g), boost::vertex(12, g), g);
    boost::add_edge(boost::vertex(16, g), boost::vertex(6, g), g);
    boost::add_edge(boost::vertex(17, g), boost::vertex(20, g), g);
    boost::add_edge(boost::vertex(18, g), boost::vertex(9, g), g);
    boost::add_edge(boost::vertex(19, g), boost::vertex(23, g), g);
    boost::add_edge(boost::vertex(19, g), boost::vertex(18, g), g);
    boost::add_edge(boost::vertex(20, g), boost::vertex(23, g), g);
    boost::add_edge(boost::vertex(20, g), boost::vertex(13, g), g);
    boost::add_edge(boost::vertex(21, g), boost::vertex(18, g), g);
    boost::add_edge(boost::vertex(21, g), boost::vertex(13, g), g);
    boost::add_edge(boost::vertex(22, g), boost::vertex(21, g), g);
    boost::add_edge(boost::vertex(23, g), boost::vertex(16, g), g);
}

template <typename Graph, typename RNGEngine>
void dijkstra_random_paths_example(
    const unsigned int start_index, const unsigned int end_index,
    const unsigned int max_weight, unsigned int num_runs)
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef boost::associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename boost::property_map<Graph,boost::vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<Vertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef boost::uniform_int<unsigned int>
            RNGDistribution;
    typedef boost::variate_generator<RNGEngine&,RNGDistribution>
            RandomNumberGenerator;

    Graph                 g(24);
    Vertex                source_vertex = boost::vertex(start_index, g);
    Vertex                target_vertex = boost::vertex(end_index, g);
    Vertex                v;
    VertexMap             v_map;
    PredecessorMap        pred_map(v_map);
    VertexIndexMap        index_map = boost::get(boost::vertex_index_t(), g);
    Path                  path;
    PathIterator          begin, end;
    RNGEngine             rng_engine;
    RNGDistribution       rng_distribution(0, max_weight);
    RandomNumberGenerator random_weight(rng_engine, rng_distribution);

    for (init_graph(g); num_runs > 0; --num_runs)
    {
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
        boost::dijkstra_random_paths(g, source_vertex, random_weight,
                                     pred_map);
#else
        boost::dijkstra_random_paths(g, source_vertex, random_weight,
                                     boost::predecessor_map(pred_map));
#endif

        for (v = target_vertex;
             v != boost::get(pred_map, v);
             v = boost::get(pred_map, v))
        {
            path.push_front(v);
        }

        path.push_front(v);
        begin = path.begin();
        end = path.end();

        std::cout << "The path: " << boost::get(index_map, *begin);

        while (++begin != end)
        {
            std::cout << " -> " << boost::get(index_map, *begin);
        }

        std::cout << std::endl;

        path.clear();
    }
}

template <typename InputGraph, typename UtilGraph, typename RNGEngine>
void ddnw_random_paths_example(
    const unsigned int start_index, const unsigned int end_index,
    const unsigned int max_weight, unsigned int num_runs)
{
    typedef typename boost::graph_traits<InputGraph>::vertex_descriptor
            InputVertex;
    typedef std::map<InputVertex,InputVertex>
            OutputVertexMap;
    typedef boost::associative_property_map<OutputVertexMap>
            OutputPredecessorMap;
    typedef typename boost::property_map<InputGraph,boost::vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<InputVertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef boost::uniform_int<unsigned int>
            RNGDistribution;
    typedef boost::variate_generator<RNGEngine&,RNGDistribution>
            RandomNumberGenerator;

    InputGraph            g(24);
    UtilGraph             dag(24);
    InputVertex           source_vertex = boost::vertex(start_index, g);
    InputVertex           target_vertex = boost::vertex(end_index, g);
    InputVertex           v;
    OutputVertexMap       out_v_map;
    OutputPredecessorMap  out_pred_map(out_v_map);
    VertexIndexMap        index_map = boost::get(boost::vertex_index_t(), g);
    Path                  path;
    PathIterator          begin, end;
    RNGEngine             rng_engine;
    RNGDistribution       rng_distribution(0, max_weight);
    RandomNumberGenerator random_weight(rng_engine, rng_distribution);

    for (init_graph(g); num_runs > 0; --num_runs)
    {
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
        boost::ddnw_random_paths(g, source_vertex, random_weight,
                                 out_pred_map, dag);
#else
        boost::ddnw_random_paths(
            g, source_vertex, random_weight, dag,
            boost::predecessor_map(out_pred_map),
            boost::vertex_index_map(boost::get(boost::vertex_index_t(), dag)));
#endif

        for (v = target_vertex;
             v != boost::get(out_pred_map, v);
             v = boost::get(out_pred_map, v))
        {
            path.push_front(v);
        }

        path.push_front(v);
        begin = path.begin();
        end = path.end();

        std::cout << "The path: " << boost::get(index_map, *begin);

        while (++begin != end)
        {
            std::cout << " -> " << boost::get(index_map, *begin);
        }

        std::cout << std::endl;

        path.clear();
    }
}

template <typename Graph, typename RNGEngine>
void loop_erased_random_paths_example(
    const unsigned int start_index, const unsigned int end_index,
    const unsigned int max_weight, unsigned int num_runs)
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef boost::associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename boost::property_map<Graph,boost::vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<Vertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef boost::random_number_generator<RNGEngine,unsigned int>
            RandomIndexGenerator;

    Graph                g(24);
    Graph                u_g(24);
    Vertex               source_vertex = boost::vertex(start_index, g);
    Vertex               target_vertex = boost::vertex(end_index, g);
    Vertex               v;
    VertexMap            v_map;
    PredecessorMap       pred_map(v_map);
    VertexIndexMap       index_map = boost::get(boost::vertex_index_t(), g);
    Path                 path;
    PathIterator         begin, end;
    RNGEngine            rng_engine;
    RandomIndexGenerator rig(rng_engine);

    init_graph(g);

    while (num_runs > 0)
    {
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
        boost::loop_erased_random_paths(g, source_vertex, rig, pred_map,
                                        u_g);
#else
        boost::loop_erased_random_paths(
            g, source_vertex, rig, u_g,
            boost::predecessor_map(pred_map),
            boost::vertex_index_map(get(boost::vertex_index_t(), u_g)));
#endif

        for (v = target_vertex;
             v != boost::get(pred_map, v);
             v = boost::get(pred_map, v))
        {
            path.push_front(v);
        }

        if (v == source_vertex)
        {
            path.push_front(v);
            begin = path.begin();
            end = path.end();

            std::cout << "The path: " << boost::get(index_map, *begin);

            while (++begin != end)
            {
                std::cout << " -> " << boost::get(index_map, *begin);
            }

            std::cout << std::endl;
            --num_runs;
        }

        path.clear();
    }
}

#ifdef RandomIndexGenerator
template <typename Graph, typename RNGEngine>
void loop_erased_random_paths_specialized_example(
    const unsigned int start_index, const unsigned int end_index,
    const unsigned int max_weight, unsigned int num_runs)
{
    typedef typename boost::graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef boost::associative_property_map<VertexMap>
            PredecessorMap;
    typedef typename boost::property_map<Graph,boost::vertex_index_t>::type
            VertexIndexMap;
    typedef std::list<Vertex>
            Path;
    typedef typename Path::iterator
            PathIterator;
    typedef boost::random_number_generator<RNGEngine,unsigned int>
            RandomIndexGenerator;

    Graph                g(24);
    Vertex               source_vertex = boost::vertex(start_index, g);
    Vertex               target_vertex = boost::vertex(end_index, g);
    Vertex               v;
    VertexMap            v_map;
    PredecessorMap       pred_map(v_map);
    VertexIndexMap       index_map = boost::get(boost::vertex_index_t(), g);
    Path                 path;
    PathIterator         begin, end;
    RNGEngine            rng_engine;
    RandomIndexGenerator rig(rng_engine);

    init_graph(g);

    while (num_runs > 0)
    {
//#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
// According to example code using dijkstra_shortest_paths,
// VC++ has trouble with the named parameters mechanism.
        boost::loop_erased_random_paths(g, source_vertex, rig, pred_map);
/*#else
        boost::loop_erased_random_paths(
            g, source_vertex, rig, boost::predecessor_map(pred_map));
#endif
*/
        for (v = target_vertex;
             v != boost::get(pred_map, v);
             v = boost::get(pred_map, v))
        {
            path.push_front(v);
        }

        if (v == source_vertex)
        {
            path.push_front(v);
            begin = path.begin();
            end = path.end();

            std::cout << "The path: " << boost::get(index_map, *begin);

            while (++begin != end)
            {
                std::cout << " -> " << boost::get(index_map, *begin);
            }

            std::cout << std::endl;
            --num_runs;
        }

        path.clear();
    }
}
#endif

template <typename UndirectedGraph, typename DirectedGraph, typename RNGEngine>
void example(const unsigned int start_index, const unsigned int end_index,
             const unsigned int max_weight, const unsigned int num_runs)
{
    std::cout << "Running dijkstra_random_paths function template on ";
    std::cout << "undirected graph." << std::endl;
    dijkstra_random_paths_example<UndirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running ddnw_random_paths function template on ";
    std::cout << "undirected graph." << std::endl;
    ddnw_random_paths_example<UndirectedGraph,DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running loop_erased_random_paths function template on ";
    std::cout << "undirected graph." << std::endl;
    loop_erased_random_paths_example<UndirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

#ifdef BOOST_SANDBOX_LERP_SPECIAL
    std::cout << "Running specialized loop_erased_random_paths function ";
    std::cout << "template on undirected graph." << std::endl;
    loop_erased_random_paths_specialized_example<UndirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;
#endif

    std::cout << "Running dijkstra_random_paths function template on ";
    std::cout << "directed graph." << std::endl;
    dijkstra_random_paths_example<DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running ddnw_random_paths function template on ";
    std::cout << "directed graph." << std::endl;
    ddnw_random_paths_example<DirectedGraph,DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running loop_erased_random_paths function template on ";
    std::cout << "directed graph." << std::endl;
    loop_erased_random_paths_example<DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
}

template <typename EdgeContainerSelector, typename RNGEngine>
void vec_adjacency_list_example(const unsigned int start_index,
                                const unsigned int end_index,
                                const unsigned int max_weight,
                                const unsigned int num_runs)
{
    typedef boost::adjacency_list<
              EdgeContainerSelector,boost::vecS,boost::undirectedS,
              boost::no_property,boost::property<boost::edge_weight_t,int>,
              boost::no_property>
            UndirectedGraph;
    typedef boost::adjacency_list<
              EdgeContainerSelector,boost::vecS,boost::directedS,
              boost::no_property,boost::property<boost::edge_weight_t,int>,
              boost::no_property>
            DirectedGraph;
    typedef boost::adjacency_list<
              EdgeContainerSelector,boost::vecS,boost::bidirectionalS,
              boost::no_property,boost::property<boost::edge_weight_t,int>,
              boost::no_property>
            BidirectionalGraph;

    example<UndirectedGraph,DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running dijkstra_random_paths function template on ";
    std::cout << "bidirectional graph." << std::endl;
    dijkstra_random_paths_example<BidirectionalGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running ddnw_random_paths function template on ";
    std::cout << "bidirectional graph." << std::endl;
    ddnw_random_paths_example<BidirectionalGraph,BidirectionalGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;

    std::cout << "Running loop_erased_random_paths function template on ";
    std::cout << "bidirectional graph." << std::endl;
    loop_erased_random_paths_example<BidirectionalGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);

#ifdef BOOST_SANDBOX_LERP_SPECIAL
    std::cout << "Running specialized loop_erased_random_paths function ";
    std::cout << "template on bidirectional graph." << std::endl;
    loop_erased_random_paths_specialized_example<BidirectionalGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
    std::cout << std::endl;
#endif
}

template <typename RNGEngine>
void adjacency_matrix_example(const unsigned int start_index,
                              const unsigned int end_index,
                              const unsigned int max_weight,
                              const unsigned int num_runs)
{
    typedef boost::adjacency_matrix<
              boost::undirectedS,boost::no_property,
              boost::property<boost::edge_weight_t,int>,boost::no_property>
            UndirectedGraph;
    typedef boost::adjacency_matrix<
              boost::directedS,boost::no_property,
              boost::property<boost::edge_weight_t,int>,boost::no_property>
            DirectedGraph;

    example<UndirectedGraph,DirectedGraph,RNGEngine>(
        start_index, end_index, max_weight, num_runs);
}

int usage()
{
    std::cout << "Options:" << std::endl;
    std::cout << "-s#, where # is an integer between 0 and 23, inclusive";
    std::cout << " (default is 0)" << std::endl;
    std::cout << "-e#, where # is an integer between 0 and 23, inclusive";
    std::cout << " (default is 22)" << std::endl;
    std::cout << "-m#, where # is an integer between 0 and 65536, inclusive";
    std::cout << " (default is 16384)" << std::endl;
    std::cout << "-n#, where # is a nonnegative integer";
    std::cout << " (default is 256)" << std::endl;
    return 0;
}

int main(int argc, char** argv)
{
    unsigned int start_index = 0;
    unsigned int end_index = 22;
    unsigned int max_weight = 16384;
    unsigned int num_runs = 256;
    char* arg;

    while (--argc > 0)
    {
        arg = argv[argc];

        if ((std::strlen(arg) > 2) && (arg[0] == '-'))
        {
            switch (arg[1])
            {
                case 's':
                    start_index = std::atoi(&arg[2]);

                    if (start_index > 23)
                    {
                        return usage();
                    }

                    break;

                case 'e':
                    end_index = std::atoi(&arg[2]);

                    if (end_index > 23)
                    {
                        return usage();
                    }

                    break;

                case 'm':
                    max_weight = std::atoi(&arg[2]);

                    if (max_weight > 65536)
                    {
                        return usage();
                    }

                    break;

                case 'n':
                    num_runs = std::atoi(&arg[2]);
                    break;

                default:
                    return usage();
            }
        }
        else
        {
            return usage();
        }
    }

//    adjacency_matrix_example<boost::mt19937>(
//    vec_adjacency_list_example<boost::multisetS,boost::mt19937>(
//    vec_adjacency_list_example<boost::setS,boost::mt19937>(
//    vec_adjacency_list_example<boost::listS,boost::mt19937>(
    vec_adjacency_list_example<boost::vecS,boost::mt19937>(
        start_index, end_index, max_weight, num_runs);

    return 0;
}

