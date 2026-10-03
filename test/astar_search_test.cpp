

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

    return boost::report_errors();
}
