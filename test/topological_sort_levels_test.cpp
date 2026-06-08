// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <algorithm>
#include <utility>
#include <vector>

#include <boost/core/lightweight_test.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/exception.hpp>
#include <boost/graph/topological_sort_levels.hpp>
#include <boost/property_map/vector_property_map.hpp>

typedef boost::adjacency_list< boost::vecS, boost::vecS, boost::directedS >
    Graph;
typedef boost::graph_traits< Graph >::vertex_descriptor Vertex;

// Check that the level assignment is consistent with the graph: for every
// edge (u, v) the level of u must be strictly less than the level of v,
// and levels are tight (any vertex above level 0 has a predecessor exactly
// one level below).
template < typename G, typename Levels >
static void check_levels_are_tight(const G& g, const Levels& levels)
{
    std::vector< std::size_t > level_of(num_vertices(g), 0);
    for (std::size_t k = 0; k < levels.size(); ++k)
    {
        for (std::size_t i = 0; i < levels[k].size(); ++i)
        {
            level_of[levels[k][i]] = k;
        }
    }

    std::size_t total = 0;
    for (std::size_t k = 0; k < levels.size(); ++k)
        total += levels[k].size();
    BOOST_TEST_EQ(total, num_vertices(g));

    typename boost::graph_traits< G >::edge_iterator ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei)
    {
        const std::size_t su = level_of[source(*ei, g)];
        const std::size_t sv = level_of[target(*ei, g)];
        BOOST_TEST_LT(su, sv);
    }

    for (std::size_t k = 1; k < levels.size(); ++k)
    {
        for (std::size_t i = 0; i < levels[k].size(); ++i)
        {
            const Vertex v = levels[k][i];
            bool has_predecessor_at_prev_level = false;
            typename boost::graph_traits< G >::edge_iterator e, e_end;
            for (boost::tie(e, e_end) = edges(g); e != e_end; ++e)
            {
                if (target(*e, g) == v && level_of[source(*e, g)] == k - 1)
                {
                    has_predecessor_at_prev_level = true;
                    break;
                }
            }
            BOOST_TEST(has_predecessor_at_prev_level);
        }
    }
}

// Vertices 0..3 with edges 0->1, 0->2, 1->3, 2->3 form a diamond.
// Following BGL's edge convention (u -> v means u precedes v in topological
// order), the expected levels are {0}, {1, 2}, {3}.
static void test_diamond()
{
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 3, g);

    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);

    BOOST_TEST_EQ(levels.size(), std::size_t(3));
    BOOST_TEST_EQ(levels[0].size(), std::size_t(1));
    BOOST_TEST_EQ(levels[0][0], Vertex(0));
    BOOST_TEST_EQ(levels[1].size(), std::size_t(2));
    BOOST_TEST(std::find(levels[1].begin(), levels[1].end(), Vertex(1))
        != levels[1].end());
    BOOST_TEST(std::find(levels[1].begin(), levels[1].end(), Vertex(2))
        != levels[1].end());
    BOOST_TEST_EQ(levels[2].size(), std::size_t(1));
    BOOST_TEST_EQ(levels[2][0], Vertex(3));

    check_levels_are_tight(g, levels);
}

static void test_empty()
{
    Graph g(0);
    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);
    BOOST_TEST_EQ(levels.size(), std::size_t(0));
}

static void test_single_vertex()
{
    Graph g(1);
    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);
    BOOST_TEST_EQ(levels.size(), std::size_t(1));
    BOOST_TEST_EQ(levels[0].size(), std::size_t(1));
    BOOST_TEST_EQ(levels[0][0], Vertex(0));
}

static void test_isolated_vertices()
{
    Graph g(5);
    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);
    BOOST_TEST_EQ(levels.size(), std::size_t(1));
    BOOST_TEST_EQ(levels[0].size(), std::size_t(5));
}

static void test_chain()
{
    Graph g(5);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 3, g);
    add_edge(3, 4, g);

    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);

    BOOST_TEST_EQ(levels.size(), std::size_t(5));
    for (std::size_t k = 0; k < levels.size(); ++k)
    {
        BOOST_TEST_EQ(levels[k].size(), std::size_t(1));
        BOOST_TEST_EQ(levels[k][0], Vertex(k));
    }
}

static void test_disconnected_components()
{
    // Two independent chains: 0->1->2 and 3->4
    Graph g(5);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(3, 4, g);

    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);

    BOOST_TEST_EQ(levels.size(), std::size_t(3));
    BOOST_TEST_EQ(levels[0].size(), std::size_t(2));
    BOOST_TEST_EQ(levels[1].size(), std::size_t(2));
    BOOST_TEST_EQ(levels[2].size(), std::size_t(1));
    BOOST_TEST_EQ(levels[2][0], Vertex(2));

    check_levels_are_tight(g, levels);
}

static void test_long_edge_skips_levels()
{
    // 0->1, 0->2, 1->2: vertex 2 has predecessors at both level 0 and level 1,
    // so it must land at level 2, not level 1.
    Graph g(3);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 2, g);

    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels);

    BOOST_TEST_EQ(levels.size(), std::size_t(3));
    BOOST_TEST_EQ(levels[0][0], Vertex(0));
    BOOST_TEST_EQ(levels[1][0], Vertex(1));
    BOOST_TEST_EQ(levels[2][0], Vertex(2));
}

static void test_cycle_throws()
{
    Graph g(3);
    add_edge(0, 1, g);
    add_edge(1, 2, g);
    add_edge(2, 0, g);

    std::vector< std::vector< Vertex > > levels;
    bool threw = false;
    try
    {
        boost::topological_sort_levels(g, levels);
    }
    catch (const boost::not_a_dag&)
    {
        threw = true;
    }
    BOOST_TEST(threw);
}

static void test_self_loop_throws()
{
    Graph g(1);
    add_edge(0, 0, g);

    std::vector< std::vector< Vertex > > levels;
    bool threw = false;
    try
    {
        boost::topological_sort_levels(g, levels);
    }
    catch (const boost::not_a_dag&)
    {
        threw = true;
    }
    BOOST_TEST(threw);
}

static void test_property_map_form()
{
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 3, g);

    boost::vector_property_map< std::size_t > level_map(num_vertices(g));
    const std::size_t num_levels = boost::topological_sort_levels(g, level_map);

    BOOST_TEST_EQ(num_levels, std::size_t(3));
    BOOST_TEST_EQ(level_map[0], std::size_t(0));
    BOOST_TEST_EQ(level_map[1], std::size_t(1));
    BOOST_TEST_EQ(level_map[2], std::size_t(1));
    BOOST_TEST_EQ(level_map[3], std::size_t(2));
}

static void test_named_param_vertex_index()
{
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 3, g);

    boost::vector_property_map< std::size_t > level_map(num_vertices(g));
    const std::size_t num_levels = boost::topological_sort_levels(g, level_map,
        boost::vertex_index_map(get(boost::vertex_index, g)));

    BOOST_TEST_EQ(num_levels, std::size_t(3));
    BOOST_TEST_EQ(level_map[0], std::size_t(0));
    BOOST_TEST_EQ(level_map[3], std::size_t(2));
}

static void test_named_param_convenience_form()
{
    Graph g(4);
    add_edge(0, 1, g);
    add_edge(0, 2, g);
    add_edge(1, 3, g);
    add_edge(2, 3, g);

    std::vector< std::vector< Vertex > > levels;
    boost::topological_sort_levels(g, levels,
        boost::vertex_index_map(get(boost::vertex_index, g)));

    BOOST_TEST_EQ(levels.size(), std::size_t(3));
    BOOST_TEST_EQ(levels[0][0], Vertex(0));
    BOOST_TEST_EQ(levels[2][0], Vertex(3));
}

int main(int, char*[])
{
    test_diamond();
    test_empty();
    test_single_vertex();
    test_isolated_vertices();
    test_chain();
    test_disconnected_components();
    test_long_edge_skips_levels();
    test_cycle_throws();
    test_self_loop_throws();
    test_property_map_form();
    test_named_param_vertex_index();
    test_named_param_convenience_form();
    return boost::report_errors();
}
