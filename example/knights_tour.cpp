//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee,
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <stdlib.h>
#include <iostream>
#include <stack>
#include <queue>
#include <ctime>
#include <boost/operators.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map/property_map.hpp>

using namespace boost;

using Position = std::pair< int, int >;
Position knight_jumps[8]
    = { Position(2, -1), Position(1, -2), Position(-1, -2), Position(-2, -1),
          Position(-2, 1), Position(-1, 2), Position(1, 2), Position(2, 1) };

Position operator+(const Position& p1, const Position& p2)
{
    return Position(p1.first + p2.first, p1.second + p2.second);
}

struct knights_tour_graph;
struct knight_adjacency_iterator
: public boost::forward_iterator_helper< knight_adjacency_iterator, Position,
      std::ptrdiff_t, Position*, Position >
{
    knight_adjacency_iterator() {}
    knight_adjacency_iterator(int ii, Position p, const knights_tour_graph& g)
    : m_pos(p), m_g(&g), m_i(ii)
    {
        valid_position();
    }
    Position operator*() const { return m_pos + knight_jumps[m_i]; }
    void operator++()
    {
        ++m_i;
        valid_position();
    }
    bool operator==(const knight_adjacency_iterator& x) const
    {
        return m_i == x.m_i;
    }

protected:
    void valid_position();
    Position m_pos;
    const knights_tour_graph* m_g;
    int m_i;
};

struct knights_tour_graph
{
    using vertex_descriptor = Position;
    using edge_descriptor = std::pair< vertex_descriptor, vertex_descriptor >;
    using adjacency_iterator = knight_adjacency_iterator;
    using out_edge_iterator = void;
    using in_edge_iterator = void;
    using edge_iterator = void;
    using vertex_iterator = void;
    using degree_size_type = int;
    using vertices_size_type = int;
    using edges_size_type = int;
    using directed_category = directed_tag;
    using edge_parallel_category = disallow_parallel_edge_tag;
    using traversal_category = adjacency_graph_tag;
    knights_tour_graph(int n) : m_board_size(n) {}
    int m_board_size;
};
int num_vertices(const knights_tour_graph& g)
{
    return g.m_board_size * g.m_board_size;
}

void knight_adjacency_iterator::valid_position()
{
    Position new_pos = m_pos + knight_jumps[m_i];
    while (m_i < 8
        && (new_pos.first < 0 || new_pos.second < 0
            || new_pos.first >= m_g->m_board_size
            || new_pos.second >= m_g->m_board_size))
    {
        ++m_i;
        new_pos = m_pos + knight_jumps[m_i];
    }
}

std::pair< knights_tour_graph::adjacency_iterator,
    knights_tour_graph::adjacency_iterator >
adjacent_vertices(
    knights_tour_graph::vertex_descriptor v, const knights_tour_graph& g)
{
    using Iter = knights_tour_graph::adjacency_iterator;
    return std::make_pair(Iter(0, v, g), Iter(8, v, g));
}

struct compare_first
{
    template < typename P > bool operator()(const P& x, const P& y)
    {
        return x.first < y.first;
    }
};

template < typename Graph, typename TimePropertyMap >
bool backtracking_search(Graph& g,
    typename graph_traits< Graph >::vertex_descriptor src,
    TimePropertyMap time_map)
{
    using Vertex = typename graph_traits< Graph >::vertex_descriptor;
    using P = std::pair< int, Vertex >;
    std::stack< P > S;
    int time_stamp = 0;

    S.push(std::make_pair(time_stamp, src));
    while (!S.empty())
    {
        Vertex x;
        boost::tie(time_stamp, x) = S.top();
        put(time_map, x, time_stamp);
        // all vertices have been visited, success!
        if (time_stamp == num_vertices(g) - 1)
            return true;

        bool deadend = true;
        typename graph_traits< Graph >::adjacency_iterator i, end;
        for (boost::tie(i, end) = adjacent_vertices(x, g); i != end; ++i)
            if (get(time_map, *i) == -1)
            {
                S.push(std::make_pair(time_stamp + 1, *i));
                deadend = false;
            }

        if (deadend)
        {
            put(time_map, x, -1);
            S.pop();
            boost::tie(time_stamp, x) = S.top();
            while (get(time_map, x) != -1)
            { // unwind stack to last unexplored vertex
                put(time_map, x, -1);
                S.pop();
                boost::tie(time_stamp, x) = S.top();
            }
        }

    } // while (!S.empty())
    return false;
}

template < typename Vertex, typename Graph, typename TimePropertyMap >
int number_of_successors(Vertex x, Graph& g, TimePropertyMap time_map)
{
    int s_x = 0;
    typename graph_traits< Graph >::adjacency_iterator i, end;
    for (boost::tie(i, end) = adjacent_vertices(x, g); i != end; ++i)
        if (get(time_map, *i) == -1)
            ++s_x;
    return s_x;
}

template < typename Graph, typename TimePropertyMap >
bool warnsdorff(Graph& g, typename graph_traits< Graph >::vertex_descriptor src,
    TimePropertyMap time_map)
{
    using Vertex = typename graph_traits< Graph >::vertex_descriptor;
    using P = std::pair< int, Vertex >;
    std::stack< P > S;
    int time_stamp = 0;

    S.push(std::make_pair(time_stamp, src));
    while (!S.empty())
    {
        Vertex x;
        boost::tie(time_stamp, x) = S.top();
        put(time_map, x, time_stamp);
        // all vertices have been visited, success!
        if (time_stamp == num_vertices(g) - 1)
            return true;

        // Put adjacent vertices into a local priority queue
        std::priority_queue< P, std::vector< P >, compare_first > Q;
        typename graph_traits< Graph >::adjacency_iterator i, end;
        int num_succ;
        for (boost::tie(i, end) = adjacent_vertices(x, g); i != end; ++i)
            if (get(time_map, *i) == -1)
            {
                num_succ = number_of_successors(*i, g, time_map);
                Q.push(std::make_pair(num_succ, *i));
            }
        bool deadend = Q.empty();
        // move vertices from local priority queue to the stack
        for (; !Q.empty(); Q.pop())
        {
            boost::tie(num_succ, x) = Q.top();
            S.push(std::make_pair(time_stamp + 1, x));
        }
        if (deadend)
        {
            put(time_map, x, -1);
            S.pop();
            boost::tie(time_stamp, x) = S.top();
            while (get(time_map, x) != -1)
            { // unwind stack to last unexplored vertex
                put(time_map, x, -1);
                S.pop();
                boost::tie(time_stamp, x) = S.top();
            }
        }

    } // while (!S.empty())
    return false;
}

struct board_map
{
    using value_type = int;
    using key_type = Position;
    using category = read_write_property_map_tag;
    board_map(int* b, int n) : m_board(b), m_size(n) {}
    friend int get(const board_map& ba, Position p);
    friend void put(const board_map& ba, Position p, int v);
    friend std::ostream& operator<<(std::ostream& os, const board_map& ba);

private:
    int* m_board;
    int m_size;
};

int get(const board_map& ba, Position p)
{
    return ba.m_board[p.first * ba.m_size + p.second];
}

void put(const board_map& ba, Position p, int v)
{
    ba.m_board[p.first * ba.m_size + p.second] = v;
}

std::ostream& operator<<(std::ostream& os, const board_map& ba)
{
    for (int i = 0; i < ba.m_size; ++i)
    {
        for (int j = 0; j < ba.m_size; ++j)
            os << get(ba, Position(i, j)) << "\t";
        os << std::endl;
    }
    return os;
}

int main(int argc, char* argv[])
{
    int N;
    if (argc == 2)
        N = atoi(argv[1]);
    else
        N = 8;

    knights_tour_graph g(N);
    int* board = new int[num_vertices(g)];
    board_map chessboard(board, N);
    for (int i = 0; i < N; ++i)
        for (int j = 0; j < N; ++j)
            put(chessboard, Position(i, j), -1);

    bool ret = warnsdorff(g, Position(0, 0), chessboard);

    if (ret)
        for (int i = 0; i < N; ++i)
        {
            for (int j = 0; j < N; ++j)
                std::cout << get(chessboard, Position(i, j)) << "\t";
            std::cout << std::endl;
        }
    else
        std::cout << "method failed" << std::endl;
    return 0;
}
