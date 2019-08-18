//=======================================================================
// Copyright 2002 Indiana University.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/config.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <utility>
#include <algorithm>

#define VERBOSE 0

#include <boost/utility.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/graph/random.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/concept/assert.hpp>

#include <boost/random/mersenne_twister.hpp>

enum vertex_id_t
{
    vertex_id = 500
};
enum edge_id_t
{
    edge_id = 501
};
namespace boost
{
BOOST_INSTALL_PROPERTY(vertex, id);
BOOST_INSTALL_PROPERTY(edge, id);
}

#include <boost/graph/adjacency_list.hpp>

using namespace boost;

/*
  This program tests models of the MutableGraph concept.
 */

using std::cerr;
using std::cout;
using std::endl;
using std::find;

template < class Graph, class Vertex, class ID >
bool check_vertex_cleared(Graph& g, Vertex v, ID id)
{
    typename graph_traits< Graph >::vertex_iterator vi, viend;
    for (boost::tie(vi, viend) = vertices(g); vi != viend; ++vi)
    {
        typename graph_traits< Graph >::adjacency_iterator ai, aiend, found;
        boost::tie(ai, aiend) = adjacent_vertices(*vi, g);
        boost::indirect_cmp< ID, std::equal_to< std::size_t > > cmp(id);

#if (defined(BOOST_MSVC) && BOOST_MSVC <= 1300) && defined(__SGI_STL_PORT)
        // seeing internal compiler errors when using std::find_if()
        found = aiend;
        for (; ai != aiend; ++ai)
            if (cmp(*ai, v))
            {
                found = ai;
                break;
            }
#elif defined(BOOST_NO_CXX98_BINDERS)
        found
            = std::find_if(ai, aiend, std::bind(cmp, v, std::placeholders::_1));
#else
        found = std::find_if(ai, aiend, std::bind1st(cmp, v));
#endif

        if (found != aiend)
        {
#if VERBOSE
            std::cerr << "should not have found vertex " << id[*found]
                      << std::endl;
#endif
            return false;
        }
    }
    return true;
}

template < class Graph, class Edge, class EdgeID >
bool check_edge_added(Graph& g, Edge e,
    typename graph_traits< Graph >::vertex_descriptor a,
    typename graph_traits< Graph >::vertex_descriptor b, EdgeID edge_id,
    std::size_t correct_id, bool inserted)
{
    if (!(source(e, g) == a))
    {
#if VERBOSE
        cerr << "    Failed, vertex a not source of e." << endl;
#endif
        return false;
    }
    else if (!(target(e, g) == b))
    {
#if VERBOSE
        cerr << "    Failed, vertex b not source of e." << endl;
#endif
        return false;
    }
    else if (!is_adjacent(g, a, b))
    {
#if VERBOSE
        cerr << "    Failed, not adj." << endl;
#endif
        return false;
    }
    else if (!in_edge_set(g, e))
    {
#if VERBOSE
        cerr << "    Failed, not in edge set." << endl;
#endif
        return false;
    }
    else if (inserted && edge_id[e] != correct_id)
    {
#if VERBOSE
        cerr << "    Failed, invalid edge property." << endl;
#endif
        return false;
    }
    else if (!inserted && edge_id[e] != edge_id[edge(a, b, g).first])
    {
#if VERBOSE
        cerr << "    Failed, invalid edge property." << endl;
#endif
        return false;
    }
    else if (num_edges(g) != count_edges(g))
    {
#if VERBOSE
        cerr << "    Failed, invalid number of edges." << endl;
#endif
        return false;
    }
    return true;
}

template < class Graph > std::size_t count_edges(Graph& g)
{
    std::size_t e = 0;
    typename boost::graph_traits< Graph >::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
        ++e;
    return e;
}

template < typename test_type_T, typename directed_type_T > void test_instance()
{

    typedef typename boost::adjacency_list< test_type_T, test_type_T,
        directed_type_T, boost::property< vertex_id_t, std::size_t >,
        boost::property< edge_id_t, std::size_t > >
        Graph;
    typedef typename boost::property< edge_id_t, std::size_t > EdgeID;

    std::size_t N = 5, E = 0;
    std::size_t old_N;

    Graph g;
    typedef typename boost::graph_traits< Graph >::vertex_descriptor Vertex;
    typedef typename boost::graph_traits< Graph >::edge_descriptor Edge;

    int i, j;
    std::size_t current_vertex_id = 0;
    std::size_t current_edge_id = 0;

    typename property_map< Graph, vertex_id_t >::type vertex_id_map
        = get(vertex_id, g);

    typename property_map< Graph, edge_id_t >::type edge_id_map
        = get(edge_id, g);

    for (std::size_t k = 0; k < N; ++k)
        add_vertex(current_vertex_id++, g);

    // also need to test EdgeIterator graph constructor -JGS
    mt19937 gen;

    for (j = 0; j < 10; ++j)
    {

        // add_edge
#if VERBOSE
        cerr << "Testing add_edge ..." << endl;
#endif
        for (i = 0; i < 6; ++i)
        {
            Vertex a, b;
            a = random_vertex(g, gen);
            do
            {
                b = random_vertex(g, gen);
            } while (a == b); // don't do self edges
#if VERBOSE
            cerr << "add_edge(" << vertex_id_map[a] << "," << vertex_id_map[b]
                 << ")" << endl;
#endif
            Edge e;
            bool inserted;
            boost::tie(e, inserted) = add_edge(a, b, current_edge_id++, g);
#if VERBOSE
            std::cout << "inserted: " << inserted << std::endl;
            std::cout << "source(e,g)" << source(e, g) << endl;
            std::cout << "target(e,g)" << target(e, g) << endl;
            std::cout << "edge_id[e] = " << edge_id_map[e] << std::endl;
            print_edges2(g, vertex_id_map, edge_id_map);
            print_graph(g, vertex_id_map);
            std::cout << "finished printing" << std::endl;
            //      print_in_edges(g, vertex_id_map);
#endif
            BOOST_ASSERT(check_edge_added(
                g, e, a, b, edge_id_map, current_edge_id - 1, inserted));
            ++E;
        }

        // remove_edge(u, v, g)
#if VERBOSE
        cerr << "Testing remove_edge(u, v, g) ..." << endl;
        is_failed = false;
#endif
        for (i = 0; i < 2; ++i)
        {
#if VERBOSE
            print_edges(g, vertex_id_map);
#endif
            Vertex a, b;

            Edge e = random_edge(g, gen);
            boost::tie(a, b) = boost::incident(e, g);
            --E;
#if VERBOSE
            cerr << "remove_edge(" << vertex_id_map[a] << ","
                 << vertex_id_map[b] << ")" << endl;
#endif
            remove_edge(a, b, g);
#if VERBOSE
            print_graph(g, vertex_id_map);
            //      print_in_edges(g, vertex_id_map);
            print_edges(g, vertex_id_map);
#endif
            BOOST_ASSERT(!is_adjacent(g, a, b));
            BOOST_ASSERT(!in_edge_set(g, a, b));
            BOOST_ASSERT(num_edges(g) == count_edges(g));
        }

#if VERBOSE
        cerr << "Testing remove_edge(e, g) ..." << endl;
        is_failed = false;
#endif
        for (i = 0; i < 2; ++i)
        {
#if VERBOSE
            print_edges(g, vertex_id_map);
#endif
            Vertex a, b;
            Edge e = random_edge(g, gen);
            boost::tie(a, b) = boost::incident(e, g);
            --E;
#if VERBOSE
            cerr << "remove_edge(" << vertex_id_map[a] << ","
                 << vertex_id_map[b] << ")" << endl;
#endif
            typename graph_traits< Graph >::edges_size_type old_E
                = num_edges(g);
            remove_edge(e, g);

#if VERBOSE
            print_graph(g, vertex_id_map);
            //      print_in_edges(g, vertex_id_map);
            print_edges(g, vertex_id_map);
#endif

            BOOST_ASSERT(old_E == num_edges(g) + 1);
            BOOST_ASSERT(num_edges(g) == count_edges(g));
        }

        // add_vertex
#if VERBOSE
        cerr << "Testing add_vertex ..." << endl;
        is_failed = false;
#endif
        old_N = num_vertices(g);
        typename graph_traits< Graph >::vertex_descriptor vid = add_vertex(g),
                                                          vidp1 = add_vertex(g);
        vertex_id_map[vid] = current_vertex_id++;
        vertex_id_map[vidp1] = current_vertex_id++;

#if VERBOSE
        print_vertices(g, vertex_id_map);
        print_graph(g, vertex_id_map);
        //    print_in_edges(g,vertex_id_map);
        print_edges(g, vertex_id_map);
#endif
        // make sure the two added vertices are in the graph's vertex set
        BOOST_ASSERT(in_vertex_set(g, vid));
        BOOST_ASSERT(in_vertex_set(g, vidp1));

        // make sure the vertices do not have any out edges yet
        typename graph_traits< Graph >::out_edge_iterator e, e_end;
        boost::tie(e, e_end) = out_edges(vid, g);
        BOOST_ASSERT(e == e_end);
        boost::tie(e, e_end) = out_edges(vidp1, g);
        BOOST_ASSERT(e == e_end);

        // make sure the vertices do not yet appear in any of the edges
        {
            typename graph_traits< Graph >::edge_iterator e, e_end;
            for (boost::tie(e, e_end) = edges(g); e != e_end; ++e)
            {
                BOOST_ASSERT(source(*e, g) != vid);
                BOOST_ASSERT(target(*e, g) != vid);
                BOOST_ASSERT(source(*e, g) != vidp1);
                BOOST_ASSERT(target(*e, g) != vidp1);
            }
        }
        // Make sure num_vertices(g) has been updated
        N = num_vertices(g);
        BOOST_ASSERT((N - 2) == old_N);

        // add_edge again
#if VERBOSE
        cerr << "Testing add_edge after add_vertex ..." << endl;
        is_failed = false;
#endif

        for (i = 0; i < 2; ++i)
        {
            Vertex a = random_vertex(g, gen), b = random_vertex(g, gen);
            while (a == vid)
                a = random_vertex(g, gen);
            while (b == vidp1)
                b = random_vertex(g, gen);
            Edge e;
            bool inserted;
#if VERBOSE
            cerr << "add_edge(" << vertex_id_map[vid] << "," << vertex_id_map[a]
                 << ")" << endl;
#endif
            boost::tie(e, inserted)
                = add_edge(vid, a, EdgeID(current_edge_id++), g);

            BOOST_ASSERT(check_edge_added(
                g, e, vid, a, edge_id_map, current_edge_id - 1, inserted));

#if VERBOSE
            cerr << "add_edge(" << vertex_id_map[b] << ","
                 << vertex_id_map[vidp1] << ")" << endl;
#endif
            // add_edge without plugin
            boost::tie(e, inserted) = add_edge(b, vidp1, g);
            if (inserted)
                edge_id_map[e] = current_edge_id;
            ++current_edge_id;

            BOOST_ASSERT(check_edge_added(
                g, e, b, vidp1, edge_id_map, current_edge_id - 1, inserted));
        }

        // clear_vertex
        Vertex c = random_vertex(g, gen);
#if VERBOSE
        cerr << "Testing clear vertex ..." << endl;
        is_failed = false;
        print_graph(g, vertex_id_map);
        //    print_in_edges(g, vertex_id_map);
        cerr << "clearing vertex " << vertex_id_map[c] << endl;
#endif
        clear_vertex(c, g);
#if VERBOSE
        print_graph(g, vertex_id_map);
        //    print_in_edges(g, vertex_id_map);
        print_edges(g, vertex_id_map);
#endif
        BOOST_ASSERT(check_vertex_cleared(g, c, vertex_id_map)
            && num_edges(g) == count_edges(g));

#if VERBOSE
        cerr << "Testing remove vertex ..." << endl;
        is_failed = false;
        cerr << "removing vertex " << vertex_id_map[c] << endl;
#endif

        old_N = num_vertices(g);
        remove_vertex(c, g);
#if VERBOSE
        print_graph(g, vertex_id_map);
        //    print_in_edges(g,vertex_id_map);
        print_edges(g, vertex_id_map);
#endif
        // can't check in_vertex_set here because the vertex_descriptor c
        // is no longer valid, we'll just make sure the vertex set has
        // one fewer vertex
        {
            typename graph_traits< Graph >::vertex_iterator v, v_end;
            boost::tie(v, v_end) = vertices(g);
            for (N = 0; v != v_end; ++v)
                ++N; // N = std::distance(v, v_end);
            BOOST_ASSERT(N == old_N - 1);
        }

        N = num_vertices(g);
        BOOST_ASSERT(N == old_N - 1);
    }
}

int main(int, char*[])
{

    test_instance< boost::vecS, boost::bidirectionalS >();
    test_instance< boost::vecS, boost::directedS >();
    test_instance< boost::vecS, boost::undirectedS >();
    test_instance< boost::listS, boost::bidirectionalS >();
    test_instance< boost::listS, boost::directedS >();
    test_instance< boost::listS, boost::undirectedS >();
    test_instance< boost::setS, boost::bidirectionalS >();
    test_instance< boost::setS, boost::directedS >();
    test_instance< boost::setS, boost::undirectedS >();

    return 0;
}
