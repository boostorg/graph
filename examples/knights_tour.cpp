//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================

#include <iostream>
#include <stack>
#include <queue>
#include <boost/operators.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/property_map.hpp>

typedef std::pair<int,int> Position;

static Position knight_jumps[8] 
  = { Position(2, -1), Position(1, -2), Position(-1, -2), Position(-2, -1),
      Position(-2, 1), Position(-1, 2), Position(1, 2  ), Position(2, 1  ) };

Position operator+(const Position& p1, const Position& p2)
{
  return Position(p1.first + p2.first, p1.second + p2.second);
}

struct knights_tour_graph;

struct knight_adjacency_iterator
  : public boost::forward_iterator_helper<
      knight_adjacency_iterator, Position, std::ptrdiff_t, Position*, Position>
{
  knight_adjacency_iterator() { }
  knight_adjacency_iterator(int ii, Position p, const knights_tour_graph& g) 
    : pos(p), m_g(&g), i(ii) { valid_position(); }
  Position operator*() const { return pos + knight_jumps[i]; }
  void operator++() { ++i; valid_position(); }
  bool operator==(const knight_adjacency_iterator& x) const 
  { return i == x.i; }
protected:
  void valid_position();
  Position pos;
  const knights_tour_graph* m_g;
  int i;
};

struct knights_tour_graph
{
  typedef Position vertex_descriptor;
  typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
  typedef knight_adjacency_iterator adjacency_iterator;
  typedef void out_edge_iterator;
  typedef void in_edge_iterator;
  typedef void edge_iterator;
  typedef void vertex_iterator;
  typedef int degree_size_type;
  typedef int vertices_size_type;
  typedef int edges_size_type;
  typedef boost::directed_tag directed_category;
  typedef boost::disallow_parallel_edge_tag edge_parallel_category;

  knights_tour_graph(int n) 
    : m_board_size(n) { }

  int m_board_size;
};

void knight_adjacency_iterator::valid_position()
{
  Position new_pos = pos + knight_jumps[i];
  while (i < 8 && (new_pos.first < 0 || new_pos.second < 0 
		   || new_pos.first >= m_g->m_board_size
		   || new_pos.second >= m_g->m_board_size)) {
    ++i;
    new_pos = pos + knight_jumps[i];
  }
}


int num_vertices(const knights_tour_graph& g) { return g.m_board_size; }

std::pair<knights_tour_graph::adjacency_iterator,
          knights_tour_graph::adjacency_iterator>
adjacent_vertices(knights_tour_graph::vertex_descriptor v, const knights_tour_graph& g)
{
  typedef knights_tour_graph::adjacency_iterator Iter;
  return std::make_pair(Iter(0, v, g), Iter(8, v, g));
}


template <class Vertex, class Graph, class Board>
int number_of_successors(Vertex x, Graph g, Board board) {
  int s_x = 0;
  typename boost::graph_traits<Graph>::adjacency_iterator i, end;
  for (boost::tie(i, end) = adjacent_vertices(x, g); i != end; ++i)
    if (get(board, *i) == -1)
      ++s_x;
  return s_x;
}

struct compare_first {
  template <class P>
  bool operator()(const P& x, const P& y) { return x.first < y.first;  }
};


template <class Graph, class TimePropertyMap>
bool backtracking_search
  (Graph& g, 
   typename boost::graph_traits<Graph>::vertex_descriptor x, 
   TimePropertyMap time_pa)
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::pair<int, Vertex> P;
  std::stack<P> S;
  int time_stamp = 0;
  S.push(std::make_pair(time_stamp, x));

  while (!S.empty()) {
    boost::tie(time_stamp, x) = S.top();
    put(time_pa, x, time_stamp);

    if (time_stamp == num_vertices(g) * num_vertices(g) - 1)
      return true;			 // success!

    bool deadend = true;
    typename boost::graph_traits<Graph>::adjacency_iterator i, end;
    for (boost::tie(i, end) = adjacent_vertices(x, g); i != end; ++i)
      if (get(time_pa, *i) == -1) {
	S.push(std::make_pair(time_stamp + 1, *i));
	deadend = false;
      }

    if (deadend) {
      put(time_pa, x, -1);
      S.pop();
      boost::tie(time_stamp, x) = S.top();
      // unwind stack to unexplored vertex
      while (get(time_pa, x) != -1) {
	put(time_pa, x, -1);
	S.pop();
	boost::tie(time_stamp, x) = S.top();
      } 
    }

  } // while (!S.empty())
  return false;
}


template <class Graph, class TimePropertyMap>
bool warnsdorff
  (Graph& g, 
   typename boost::graph_traits<Graph>::vertex_descriptor src, 
   TimePropertyMap time_pa)
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
  typedef std::pair<int, Vertex> P;
  std::stack<P> S;
  std::priority_queue<P, std::vector<P>, compare_first> Q;
  int time_stamp = 0;
  S.push(std::make_pair(time_stamp, src));

  while (!S.empty()) {
    Vertex x;
    boost::tie(time_stamp, x) = S.top();
    put(time_pa, x, time_stamp);
    S.pop();

    if (time_stamp == num_vertices(g) * num_vertices(g) - 1)
      return true;			 // success!

    typename boost::graph_traits<Graph>::adjacency_iterator i, end;
    boost::tie(i, end) = adjacent_vertices(x, g);
    int num_succ;
    for (; i != end; ++i)
      if (get(time_pa, *i) == -1) {
	num_succ = number_of_successors(*i, g, time_pa);
	Q.push(std::make_pair(num_succ, *i));
      }

    if (Q.empty()) {			 // hit dead-end
      put(time_pa, x, -1);
      boost::tie(time_stamp, x) = S.top();
      while (get(time_pa, x) != -1) { // unwind stack
	put(time_pa, x, -1);
	S.pop();
	boost::tie(time_stamp, x) = S.top();
      } 
    } else {
      for (; !Q.empty(); Q.pop()) {
	boost::tie(num_succ, x) = Q.top();
	S.push(std::make_pair(time_stamp + 1, x));
      }
    }

  } // while (!S.empty())
  return false;
}


struct board_map {
  typedef int value_type;
  typedef Position key_type;
  typedef boost::read_write_property_map_tag category;
  board_map(int* b, int n) : m_board(b), m_size(n) { }
  friend int get(const board_map& ba, Position p);
  friend void put(const board_map& ba, Position p, int v);
  friend std::ostream& operator<<(std::ostream& os, const board_map& ba);
private:
  int* m_board;
  int m_size;
};

int get(const board_map& ba, Position p) {
  return ba.m_board[p.first * ba.m_size + p.second];
}
void put(const board_map& ba, Position p, int v) {
  ba.m_board[p.first * ba.m_size + p.second] = v;
}

std::ostream& operator<<(std::ostream& os, const board_map& ba) {
  for (int i = 0; i < ba.m_size; ++i) {
    for (int j = 0; j < ba.m_size; ++j)
      os << get(ba, Position(i,j)) << "\t";
    os << std::endl;
  }
  return os;
}

int
main(int argc, char* argv[])
{
  int N;
  if (argc == 2)
    N = atoi(argv[1]);
  else
    N = 8;

  knights_tour_graph g(N);
  int* board = new int[num_vertices(g) * num_vertices(g)];

  board_map chessboard(board, num_vertices(g));

  for (int i = 0; i < num_vertices(g); ++i)
    for (int j = 0; j < num_vertices(g); ++j)
      put(chessboard, Position(i,j), -1);

  bool ret = warnsdorff(g, Position(0,0), chessboard);
  
  if (ret)
    for (int i = 0; i < num_vertices(g); ++i) {
      for (int j = 0; j < num_vertices(g); ++j)
	std::cout << get(chessboard, Position(i,j)) << "\t";
      std::cout << std::endl;
    }
  else
    std::cout << "method failed" << std::endl;
}

/*

(* Improved Warnsdorff's Algorithm for the Problem of  *)
(* the Knight                                          *)
(* Mathematica 2.2 Version Copyright 1992-93 Arnd Roth *)

knightJumps = {{2, -1}, {1, -2}, {-1, -2}, {-2, -1},
               {-2, 1}, {-1, 2}, {1, 2  }, {2, 1  }};

successors[s_, n_, position_] :=
Block[{moves, onBoard},
      moves = Map[position + # &, knightJumps];
      (* moves over the edge of the chessboard are *)
      (* invalid *)
      onBoard = Select[moves, And @@ Thread[
                              {0, 0} < # <= {n, n}] &];
      Select[onBoard, s[[Sequence @@ #]] == 0 &]
]

numberOfSuccessors[s_, n_, position_] :=
Length[successors[s, n, position]]

mina[s_, n_, position_] :=
(* successor(s) with least number of successors *)
Block[{localSuccessors, localNumbersOfSuccessors,
       minimum},
      localSuccessors = successors[s, n, position];
      localNumbersOfSuccessors =
      Map[numberOfSuccessors[s, n, #] &,
          localSuccessors];
      minimum = Min[localNumbersOfSuccessors];
      localSuccessors[[Flatten[Positionition[
      localNumbersOfSuccessors, minimum]]]]
]

mabst[destination_, successorlist_] :=
(* successor(s) with greatest distance from *)
(* destination *)
Block[{vectors, distances, maximum},
      vectors = Map[destination - # &, successorlist];
      distances = Map[# . # &, vectors];
      maximum = Max[distances];
      successorlist[[Flatten[Positionition[
      distances, maximum]]]]
]

improvedWarnsdorff[n_] :=
(* main program *)
Block[{s, destination, position, localMina},
      (* initialize chessboard s *)
      (* s[[x, y]] == 0  ==> square (x, y) is *)
      (* untouched. *)
      (* in the end, s contains the step numbers *)
      (* at which every square was visited *)
      s = Table[0, {n}, {n}];
      (* path should end near the following *)
      destination =
      {Round[n / 2] - 1 / 2, Round[n / 2] - 1 / 2};
      (* starting point in the corner of the board *)
      position = {1, 1};
      s[[1, 1]] = 1;
      Do[
         localMina = mina[s, n, position];
         If[Length[localMina] == 0,
            Print["Blind alley"];
            (* algorithm failed *)
            Break[]
         ];
         position = If[Length[localMina] == 1,
                       First[localMina],
                       First[mabst[destination,
                                   localMina]]
                    ];
         s[[First[position], Last[position]]] =
         stepNumber,
      {stepNumber, 2, n n}];
      s
]     s

*/


#if 0
template <class Vertex, class Graph, class Board>
bool recursive_backtracking(Vertex x, Graph& g, Board chessboard, int t)
{
  put(chessboard, x, t);
  if (t == num_vertices(g) * num_vertices(g) - 1)
    return true; // success!

  typename boost::graph_traits<Graph>::adjacency_iterator i, end;
  boost::tie(i, end) = adjacent_vertices(x, g);
  for (; i != end; ++i)
    if (get(chessboard, *i) == -1
	&& recursive_backtracking(*i, g, chessboard, t + 1))
      return true;
  put(chessboard, x, -1);
  return false;
}
template <class Graph, class Board>
bool backtracking(Graph& g, Board chessboard)
{
  Position x(0,0);
  int t = 0;
  return recursive_backtracking(x, g, chessboard, t);
}



template <class Vertex, class Graph, class Board>
bool recursive_warnsdorff(Vertex u, Graph& g, Board chessboard, int t)
{
  typename boost::graph_traits<Graph>::adjacency_iterator i, end;
  typedef std::pair<int,Vertex> P;
  std::priority_queue<P, std::vector<P>, compare_first> Q;
  int num_succ;

  put(chessboard, u, t);
  if (t == num_vertices(g) * num_vertices(g) - 1)
    return true; // success!

  for (boost::tie(i, end) = adjacent_vertices(u, g); i != end; ++i)
    if (get(chessboard, *i) == -1) {
      num_succ = number_of_successors(*i, g, chessboard);
      Q.push(std::make_pair(num_succ, *i));
    }
  for (; !Q.empty(); Q.pop()) {
    Vertex v;
    boost::tie(num_succ, v) = Q.top();
    if (recursive_warnsdorff(v, g, chessboard, t + 1))
      return true;
  }
  put(chessboard, u, -1);
  return false;
}
template <class Graph, class Board>
bool warnsdorff(Graph& g, Board chessboard)
{
  Position start(0,0);
  int t = 0;
  return recursive_warnsdorff(start, g, chessboard, t);
}

#endif
