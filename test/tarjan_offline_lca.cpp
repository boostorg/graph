/* tarjan_offline_lca.cpp source file
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
 * Defines the std::map class template.
 */
#include <map>

/*
 * Defines the boost::associative_property_map class template.
 */
#include <boost/property_map.hpp>

/*
 * Defines the boost::numeric::ublas::matrix class template.
 */
#include <boost/numeric/ublas/matrix.hpp>

/*
 * Defines the boost::graph_traits class template.
 */
#include <boost/graph/graph_traits.hpp>

/*
 * Defines the graph selector tags.
 */
#include <boost/graph/graph_selectors.hpp>

/*
 * Defines the BGL named parameters.
 */
#include <boost/graph/named_function_params.hpp>

/*
 * Defines the boost::depth_first_search and boost::make_dfs_visitor function
 * templates.
 */
#include <boost/graph/depth_first_search.hpp>

/*
 * Defines the boost::is_descendant function template.
 */
#include <boost/graph/graph_utility.hpp>

/*
 * Defines the boost::adjacency_list class template and its associated nonmember
 * function templates.
 */
#include <boost/graph/adjacency_list.hpp>

/*
 * Defines the boost::tarjan_offline_lca function template.
 */
#include <boost/graph/tarjan_offline_lca.hpp>

using boost::unit_test_framework::test_suite;
using namespace boost;

template <typename PredecessorMap>
void check_lca(
    typename property_traits<PredecessorMap>::value_type ancestor,
    typename property_traits<PredecessorMap>::value_type descendant1,
    typename property_traits<PredecessorMap>::value_type descendant2,
    PredecessorMap pred_map)
{
    if (ancestor == descendant1)
    {
        if (ancestor != descendant2)
        {
            BOOST_CHECK_MESSAGE(
                is_descendant(descendant2, ancestor, pred_map),
                "Vertex " << descendant2 << " is not reachable from "
                          << ancestor << "!");
        }
    }
    else if (ancestor == descendant2)
    {
        BOOST_CHECK_MESSAGE(
            is_descendant(descendant1, ancestor, pred_map),
            "Vertex " << descendant1 << " is not reachable from "
                      << ancestor << "!");
    }
    else if (is_descendant(descendant1, ancestor, pred_map))
    {
        if (is_descendant(descendant2, ancestor, pred_map))
        {
            typename property_traits<PredecessorMap>::value_type
                v = get(pred_map, descendant1);

            while (v != ancestor)
            {
                if (is_descendant(descendant2, v, pred_map))
                {
                    BOOST_ERROR("Vertex " << v << " is the lca of "
                                          << descendant1 << " and "
                                          << descendant2 << ", not "
                                          << ancestor << "!");
                    return;
                }

                v = get(pred_map, v);
            }
        }
        else
        {
            BOOST_ERROR("Vertex " << descendant2 << " is not reachable from "
                                  << ancestor << "!");
        }
    }
    else
    {
        BOOST_ERROR("Vertex " << descendant1 << " is not reachable from "
                              << ancestor << "!");
    }
}

template <typename Graph>
void test()
{
    typedef typename graph_traits<Graph>::vertex_descriptor
            Vertex;
    typedef std::map<Vertex,Vertex>
            VertexMap;
    typedef associative_property_map<VertexMap>
            PredecessorMap;

    const int vertex_count = 15;

    Graph g(vertex_count);

    boost::add_edge(0, 1, g);
    boost::add_edge(0, 2, g);
    boost::add_edge(1, 3, g);
    boost::add_edge(1, 4, g);
    boost::add_edge(2, 5, g);
    boost::add_edge(2, 6, g);
    boost::add_edge(3, 7, g);
    boost::add_edge(3, 8, g);
    boost::add_edge(4, 9, g);
    boost::add_edge(4, 10, g);
    boost::add_edge(5, 11, g);
    boost::add_edge(5, 12, g);
    boost::add_edge(6, 13, g);
    boost::add_edge(6, 14, g);

    numeric::ublas::matrix<unsigned int>
        ancestor_matrix(vertex_count, vertex_count);

    tarjan_offline_lca(g, 0, ancestor_matrix);

    VertexMap      v_map;
    PredecessorMap p_map(v_map);

    depth_first_search(
        g, root_vertex(0).
           visitor(make_dfs_visitor(record_predecessors(p_map,
                                                        on_tree_edge()))));

    for (int i = 0; i < vertex_count; ++i)
    {
        for (int j = 0; j < vertex_count; ++j)
        {
            check_lca(ancestor_matrix(i, j), i, j, p_map);
        }
    }
}

/*
void undirected_graph_test_case()
{
    BOOST_MESSAGE("Testing tarjan_offline_lca using undirected graph.");
    test<adjacency_list<listS,vecS,undirectedS> >();
}
*/

void directed_graph_test_case()
{
    BOOST_MESSAGE("Testing tarjan_offline_lca using directed graph.");
    test<adjacency_list<listS,vecS,directedS> >();
}

void bidirectional_graph_test_case()
{
    BOOST_MESSAGE("Testing tarjan_offline_lca using bidirectional graph.");
    test<adjacency_list<listS,vecS,bidirectionalS> >();
}

test_suite* init_unit_test_suite(int argc, char** argv)
{
    test_suite* test = BOOST_TEST_SUITE("tarjan_offline_lca unit test");

//    test->add(BOOST_TEST_CASE(&undirected_graph_test_case));
    test->add(BOOST_TEST_CASE(&directed_graph_test_case));
    test->add(BOOST_TEST_CASE(&bidirectional_graph_test_case));

    return test;
}

