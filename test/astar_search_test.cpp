

//
//=======================================================================
// Copyright (c) 2004 Kristopher Beevers
// Copyright (c) 2026 Arnaud Becheler
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//

#include <boost/graph/astar_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/core/lightweight_test.hpp>

#include <cstddef>
#include <functional>
#include <utility>
#include <vector>
#include <list>
#include <limits>
#include <math.h> // for sqrt

// auxiliary types
struct location
{
    float y, x; // lat, long
};

struct my_float
{
    float v;
    explicit my_float(float v = float()) : v(v) {}
};

using cost = my_float;
my_float operator+(my_float a, my_float b) { return my_float(a.v + b.v); }
bool operator==(my_float a, my_float b) { return a.v == b.v; }
bool operator<(my_float a, my_float b) { return a.v < b.v; }

// euclidean distance heuristic
template < class Graph, class CostType, class LocMap >
class distance_heuristic : public boost::astar_heuristic< Graph, CostType >
{
public:
    using Vertex = typename boost::graph_traits< Graph >::vertex_descriptor;
    distance_heuristic(LocMap l, Vertex goal) : m_location(l), m_goal(goal) {}
    CostType operator()(Vertex u)
    {
        float dx = m_location[m_goal].x - m_location[u].x;
        float dy = m_location[m_goal].y - m_location[u].y;
        return CostType(::sqrt(dx * dx + dy * dy));
    }

private:
    LocMap m_location;
    Vertex m_goal;
};

struct found_goal
{
}; // exception for termination

// visitor that terminates when we find the goal
template < class Vertex >
class astar_goal_visitor : public boost::default_astar_visitor
{
public:
    astar_goal_visitor(Vertex goal) : m_goal(goal) {}
    template < class Graph > void examine_vertex(Vertex u, Graph&)
    {
        if (u == m_goal)
            throw found_goal();
    }

private:
    Vertex m_goal;
};

// state in a plain data member, so it survives only through std::ref
struct examine_tally : boost::default_astar_visitor
{
    template < class Vertex, class Graph > void examine_vertex(Vertex, Graph&)
    {
        ++count;
    }
    std::size_t count = 0;
};

// a zero heuristic keeps the search deterministic, A* then behaves like
// dijkstra and examines every reachable vertex once
template < class Graph >
struct zero_heuristic : boost::astar_heuristic< Graph, int >
{
    using Vertex = typename boost::graph_traits< Graph >::vertex_descriptor;
    int operator()(Vertex) { return 0; }
};

void test_stateful_visitor_with_ref()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property< boost::edge_weight_t, int > >;
    graph_t g(3);
    boost::add_edge(0, 1, 1, g);
    boost::add_edge(1, 2, 1, g);

    std::vector< std::size_t > parent(boost::num_vertices(g));
    std::vector< int > rank(boost::num_vertices(g));
    std::vector< int > distance(boost::num_vertices(g));
    std::vector< boost::default_color_type > color(boost::num_vertices(g));

    auto index_map = boost::get(boost::vertex_index, g);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto rank_map = boost::make_iterator_property_map(rank.begin(), index_map);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    examine_tally tracked;
    boost::astar_search(g, 0, zero_heuristic< graph_t >(), std::ref(tracked),
        parent_map, rank_map, distance_map, boost::get(boost::edge_weight, g),
        index_map, color_map, std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);
    BOOST_TEST_EQ(tracked.count, boost::num_vertices(g));
    BOOST_TEST_EQ(distance[2], 2);

    // by value the caller's visitor is left untouched
    examine_tally copied;
    boost::astar_search(g, 0, zero_heuristic< graph_t >(), copied, parent_map,
        rank_map, distance_map, boost::get(boost::edge_weight, g), index_map,
        color_map, std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);
    BOOST_TEST_EQ(copied.count, static_cast< std::size_t >(0));
}

// counts the two relaxation outcomes the search can reach on an edge whose
// target has already been discovered or finished
struct branch_tally : boost::default_astar_visitor
{
    template < class Edge, class Graph > void edge_not_relaxed(Edge, Graph&)
    {
        ++not_relaxed;
    }

    template < class Edge, class Graph > void black_target(Edge, Graph&)
    {
        ++reopened;
    }

    std::size_t not_relaxed = 0;
    std::size_t reopened = 0;
};

// an overestimating heuristic, which is what makes A* reopen a vertex it has
// already finished
template < class Graph >
struct inconsistent_heuristic : boost::astar_heuristic< Graph, int >
{
    using Vertex = typename boost::graph_traits< Graph >::vertex_descriptor;
    int operator()(Vertex u) { return u == static_cast< Vertex >(2) ? 100 : 0; }
};

void test_edge_not_relaxed_on_discovered_target()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property< boost::edge_weight_t, int > >;

    // the long 1 to 2 edge never improves either endpoint
    graph_t g(3);
    boost::add_edge(0, 1, 1, g);
    boost::add_edge(0, 2, 1, g);
    boost::add_edge(1, 2, 5, g);

    std::vector< std::size_t > parent(boost::num_vertices(g));
    std::vector< int > rank(boost::num_vertices(g));
    std::vector< int > distance(boost::num_vertices(g));
    std::vector< boost::default_color_type > color(boost::num_vertices(g));

    auto index_map = boost::get(boost::vertex_index, g);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto rank_map = boost::make_iterator_property_map(rank.begin(), index_map);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    branch_tally tracked;
    boost::astar_search(g, 0, zero_heuristic< graph_t >(), std::ref(tracked),
        parent_map, rank_map, distance_map, boost::get(boost::edge_weight, g),
        index_map, color_map, std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);

    BOOST_TEST_EQ(distance[1], 1);
    BOOST_TEST_EQ(distance[2], 1);
    BOOST_TEST(tracked.not_relaxed > 0u);
    BOOST_TEST_EQ(tracked.reopened, static_cast< std::size_t >(0));
}

void test_black_target_reopens_finished_vertex()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::directedS, boost::no_property,
        boost::property< boost::edge_weight_t, int > >;

    // vertex 3 is finished through 1 at distance 11, then the detour through
    // the penalised vertex 2 offers it at distance 3
    graph_t g(4);
    boost::add_edge(0, 1, 1, g);
    boost::add_edge(1, 3, 10, g);
    boost::add_edge(0, 2, 2, g);
    boost::add_edge(2, 3, 1, g);

    std::vector< std::size_t > parent(boost::num_vertices(g));
    std::vector< int > rank(boost::num_vertices(g));
    std::vector< int > distance(boost::num_vertices(g));
    std::vector< boost::default_color_type > color(boost::num_vertices(g));

    auto index_map = boost::get(boost::vertex_index, g);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto rank_map = boost::make_iterator_property_map(rank.begin(), index_map);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    branch_tally tracked;
    boost::astar_search(g, 0, inconsistent_heuristic< graph_t >(),
        std::ref(tracked), parent_map, rank_map, distance_map,
        boost::get(boost::edge_weight, g), index_map, color_map,
        std::less< int >(), boost::closed_plus< int >(),
        (std::numeric_limits< int >::max)(), 0);

    BOOST_TEST_EQ(tracked.reopened, static_cast< std::size_t >(1));
    BOOST_TEST_EQ(distance[3], 3);
}

void test_edge_not_relaxed_on_tree_edge()
{
    using graph_t = boost::adjacency_list< boost::vecS, boost::vecS,
        boost::directedS, boost::no_property,
        boost::property< boost::edge_weight_t, int > >;

    // two hops of 60 overshoot the infinity below, so the second hop never
    // improves on it and vertex 2 stays unreached
    graph_t g(3);
    boost::add_edge(0, 1, 60, g);
    boost::add_edge(1, 2, 60, g);

    const int infinity = 100;

    std::vector< std::size_t > parent(boost::num_vertices(g));
    std::vector< int > rank(boost::num_vertices(g));
    std::vector< int > distance(boost::num_vertices(g));
    std::vector< boost::default_color_type > color(boost::num_vertices(g));

    auto index_map = boost::get(boost::vertex_index, g);
    auto parent_map
        = boost::make_iterator_property_map(parent.begin(), index_map);
    auto rank_map = boost::make_iterator_property_map(rank.begin(), index_map);
    auto distance_map
        = boost::make_iterator_property_map(distance.begin(), index_map);
    auto color_map
        = boost::make_iterator_property_map(color.begin(), index_map);

    branch_tally tracked;
    boost::astar_search(g, 0, zero_heuristic< graph_t >(), std::ref(tracked),
        parent_map, rank_map, distance_map, boost::get(boost::edge_weight, g),
        index_map, color_map, std::less< int >(),
        boost::closed_plus< int >(infinity), infinity, 0);

    BOOST_TEST_EQ(distance[1], 60);
    BOOST_TEST_EQ(distance[2], infinity);
    BOOST_TEST_EQ(tracked.not_relaxed, static_cast< std::size_t >(1));
    BOOST_TEST_EQ(tracked.reopened, static_cast< std::size_t >(0));
}

int main()
{
    // specify some types
    using mygraph_t = boost::adjacency_list< boost::listS, boost::vecS,
        boost::undirectedS, boost::no_property,
        boost::property< boost::edge_weight_t, cost > >;
    using WeightMap = boost::property_map< mygraph_t, boost::edge_weight_t >::type;
    using vertex = mygraph_t::vertex_descriptor;
    using edge_descriptor = mygraph_t::edge_descriptor;
    using edge = std::pair< int, int >;

    // specify data
    enum nodes
    {
        Troy,
        LakePlacid,
        Plattsburgh,
        Massena,
        Watertown,
        Utica,
        Syracuse,
        Rochester,
        Buffalo,
        Ithaca,
        Binghamton,
        Woodstock,
        NewYork,
        N
    };
    location locations[] = { // lat/long
        { 42.73, 73.68 }, { 44.28, 73.99 }, { 44.70, 73.46 }, { 44.93, 74.89 },
        { 43.97, 75.91 }, { 43.10, 75.23 }, { 43.04, 76.14 }, { 43.17, 77.61 },
        { 42.89, 78.86 }, { 42.44, 76.50 }, { 42.10, 75.91 }, { 42.04, 74.11 },
        { 40.67, 73.94 }
    };
    edge edge_array[]
        = { edge(Troy, Utica), edge(Troy, LakePlacid), edge(Troy, Plattsburgh),
              edge(LakePlacid, Plattsburgh), edge(Plattsburgh, Massena),
              edge(LakePlacid, Massena), edge(Massena, Watertown),
              edge(Watertown, Utica), edge(Watertown, Syracuse),
              edge(Utica, Syracuse), edge(Syracuse, Rochester),
              edge(Rochester, Buffalo), edge(Syracuse, Ithaca),
              edge(Ithaca, Binghamton), edge(Ithaca, Rochester),
              edge(Binghamton, Troy), edge(Binghamton, Woodstock),
              edge(Binghamton, NewYork), edge(Syracuse, Binghamton),
              edge(Woodstock, Troy), edge(Woodstock, NewYork) };
    unsigned int num_edges = sizeof(edge_array) / sizeof(edge);
    cost weights[] = { // estimated travel time (mins)
        my_float(96), my_float(134), my_float(143), my_float(65), my_float(115),
        my_float(133), my_float(117), my_float(116), my_float(74), my_float(56),
        my_float(84), my_float(73), my_float(69), my_float(70), my_float(116),
        my_float(147), my_float(173), my_float(183), my_float(74), my_float(71),
        my_float(124)
    };

    // create graph
    mygraph_t g(N);
    WeightMap weightmap = boost::get(boost::edge_weight, g);
    for (std::size_t j = 0; j < num_edges; ++j)
    {
        edge_descriptor e;
        bool inserted;
        boost::tie(e, inserted)
            = boost::add_edge(edge_array[j].first, edge_array[j].second, g);
        weightmap[e] = weights[j];
    }

    // Troy to Buffalo has a unique shortest path of 309 minutes
    vertex start = Troy;
    vertex goal = Buffalo;
    constexpr float expected_time = 309.0f;

    std::vector< mygraph_t::vertex_descriptor > p(boost::num_vertices(g));
    std::vector< cost > d(boost::num_vertices(g));

    boost::property_map< mygraph_t, boost::vertex_index_t >::const_type idx
        = boost::get(boost::vertex_index, g);

    bool found = false;
    try
    {
        // call astar named parameter interface
        boost::astar_search(g, start,
            distance_heuristic< mygraph_t, cost, location* >(locations, goal),
            boost::predecessor_map(
                boost::make_iterator_property_map(p.begin(), idx))
                .distance_map(boost::make_iterator_property_map(d.begin(), idx))
                .visitor(astar_goal_visitor< vertex >(goal))
                .distance_inf(my_float((std::numeric_limits< float >::max)())));
    }
    catch (found_goal const&)
    {
        found = true;
    }

    // the goal is reachable and the reported cost is optimal
    BOOST_TEST(found);
    BOOST_TEST_EQ(d[goal].v, expected_time);

    // the predecessor path is connected and its weights sum to the distance
    std::list< vertex > shortest_path;
    for (vertex v = goal;; v = p[v])
    {
        shortest_path.push_front(v);
        if (p[v] == v)
            break;
    }
    BOOST_TEST(shortest_path.front() == start);
    BOOST_TEST(shortest_path.back() == goal);

    cost path_weight;
    bool first = true;
    vertex prev = start;
    for (vertex v : shortest_path)
    {
        if (!first)
        {
            std::pair< edge_descriptor, bool > e = boost::edge(prev, v, g);
            BOOST_TEST(e.second);
            if (e.second)
                path_weight = path_weight + weightmap[e.first];
        }
        prev = v;
        first = false;
    }
    BOOST_TEST_EQ(path_weight.v, d[goal].v);

    test_stateful_visitor_with_ref();
    test_edge_not_relaxed_on_discovered_target();
    test_edge_not_relaxed_on_tree_edge();
    test_black_target_reopens_finished_vertex();

    return boost::report_errors();
}
