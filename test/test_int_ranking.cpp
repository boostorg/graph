//=======================================================================
// Copyright 2015 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// This test first creates a complete graph with (n+1) vertices
// ranging from 0 to n, and with n being the root vertex.  Each
// arc is assigned a random integer weight between -10 and 10.  The test
// then constructs all potential branchings and discards those
// that have indegree for any vertex greater than one or contain
// a cycle.  The valid branchings are then stored in a priority
// queue with smallest weight branching at the top.  The test then
// uses the rank_spanning_branchings.hpp routine to construct all
// branchings, in order, and places them in a vector.  The vector
// and the priority queue are then compared, allowing for the possibility
// of equal weight branchings.

#include <iostream>
#include <vector>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/assert.hpp>

#include <boost/graph/rank_spanning_branchings.hpp>

using namespace boost;

struct my_compare
{
  template<typename T>
  bool operator()( const T& w1, const T& w2 ) const
  {
    return w1 > w2;
  }
};

template<typename Edge>
struct Branching
{
  int                         weight;
  unordered_set<Edge>  v_edges;
  Branching( int w, const unordered_set<Edge>& v_e ) :
    weight( w ), v_edges( v_e ){}
  bool operator<( Branching const & rhs ) const
  { return my_compare()( weight, rhs.weight ); }
};

template<class Graph, class Edge>
struct set_rank_vector
{

  const Graph& m_g;
  std::vector<Branching<Edge> >& rank_vector;
  typedef typename property_map<Graph, edge_weight_t>::const_type WeightMap;
  WeightMap w;
  typename property_traits<WeightMap>::value_type weight;
  unordered_set<Edge> branching;

  set_rank_vector(
    const Graph& g,
    std::vector<Branching<Edge> >& rv
  ) : m_g( g ), rank_vector( rv ) {}

  template<class EdgeIterator>
  bool operator()( EdgeIterator it, const EdgeIterator end )
  {

    w = get( edge_weight, m_g );

    weight = 0;

    while( it != end )
    {

      weight += get( w, *it );

      branching.insert( *it++ );

    }

    rank_vector.push_back( Branching<Edge>( weight, branching ) ); 

    return true;

  }

};

template<class Graph, class Edge, class Rank, class Parent>
void
set_branching(
  Graph& g,
  const std::vector<Edge>& v_edges,
  const std::vector<size_t>& combination,
  Rank rank,
  Parent parent,
  heap::fibonacci_heap<Branching<Edge> >& branching_heap
)
{
  disjoint_sets<Rank, Parent> W( rank, parent );
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  unordered_set<Vertex> in_vertex_set;
  typename property_map<Graph, edge_weight_t>::type weightmap =
    get(edge_weight, g);
  unordered_set<Edge> branching_edges;

  BGL_FORALL_VERTICES_T( v, g, Graph )
  {
    W.make_set( v );
  }

  BOOST_FOREACH( size_t i, combination )
  {
    Edge e = v_edges[i];
    if( in_vertex_set.find( target( e, g ) ) != in_vertex_set.end() ) return;
    if( W.find_set( source( e, g ) ) == W.find_set( target( e, g ) ) ) return;
    W.union_set( source( e, g ), target( e, g ) );
    in_vertex_set.insert( target( e, g ) );
  }

  int weight = 0;
  BOOST_FOREACH( size_t i, combination )
  {
    Edge e = v_edges[i];
    weight += weightmap[e];
    branching_edges.insert( e );
  }
  branching_heap.push( Branching<Edge>( weight, branching_edges ) );
}

template<class Graph, class Edge>
void
loop(
  size_t offset,
  size_t k,
  Graph& g,
  const std::vector<Edge>& v_edges,
  const std::vector<size_t>& indices,
  std::vector<size_t>& combination,
  heap::fibonacci_heap<Branching<Edge> >& branching_heap
)
{

  typedef typename graph_traits<Graph>::vertices_size_type size_type;
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;

  size_type n = num_vertices( g );
  std::vector<size_type> rank_map( n );
  std::vector<vertex_t> pred_map( n );

  if (k == 0) {
    set_branching(
      g,
      v_edges,
      combination,
      make_iterator_property_map(
        rank_map.begin(),
        get(vertex_index, g),
        rank_map[0]
      ),
      make_iterator_property_map(
        pred_map.begin(),
        get(vertex_index, g),
        pred_map[0]
      ),
      branching_heap
    );
    return;
  }
  for (size_t i = offset; i <= indices.size() - k; ++i) {
    combination.push_back(indices[i]);
    loop(
      i+1,
      k-1,
      g,
      v_edges,
      indices,
      combination,
      branching_heap
    );
    combination.pop_back();
  }
}

int main( int argc, char **argv )
{

  typedef adjacency_list < listS, setS, directedS,
     property < vertex_index_t, size_t >, property < edge_weight_t, int > >
     Graph;
  typedef property_map < Graph, vertex_index_t >::type index_map_t;
  typedef graph_traits <Graph>::edge_descriptor Edge;
  heap::fibonacci_heap<Branching<Edge> > branching_heap;
  typedef random::mt19937 base_generator_type;

  std::vector<size_t> indices;
  std::vector<size_t> combination;
  std::vector<Edge> v_edges;

  base_generator_type gen;
  gen.seed( (unsigned int) time( NULL ) );

  random::uniform_int_distribution<> tdist( -10, 10 );

  variate_generator<
    base_generator_type&, random::uniform_int_distribution<>
  > rvt( gen, tdist );

  Graph g;

  if( argc != 2 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] << " n" << std::endl << std::endl;
    std::cerr << "    n = number of vertices in complete graph" <<
                 std::endl << std::endl;
    return EXIT_FAILURE;
  }

  size_t n = lexical_cast<size_t>( argv[1] );

  for( size_t i = 0; i <= n; i++ )
  {
    add_vertex( g );
  }

  // Set index map.

  index_map_t index_map = get( vertex_index, g );

  size_t j = 0;
  BGL_FORALL_VERTICES( v, g, Graph )
  {
    put( index_map, v, j++ );
  }

  // Add edges.

  BGL_FORALL_VERTICES( v, g, Graph )
  {
    BGL_FORALL_VERTICES( u, g, Graph )
    {
      if( u != v )
      {
        if( index_map[v] != 0 )
        {
          if( index_map[u] != 0 )
          {
            add_edge( v, u, rvt(), g );
          }
        }
        else
        {
          add_edge( v, u, 0, g );
        }
      }
    }
  }

  // Create vectors.

  BGL_FORALL_EDGES( e, g, Graph )
  {
    v_edges.push_back( e );
  }

  for( size_t i = 0; i < v_edges.size(); ++i )
  {
    indices.push_back(i);
  }

  loop(
    0,
    num_vertices(g) - 1,
    g,
    v_edges,
    indices,
    combination,
    branching_heap
  );

  std::vector<Branching<Edge> > rank_vector;

  rank_spanning_branchings(
    g,
    set_rank_vector<Graph,Edge>( g, rank_vector ),
    my_compare()
  );

  assert(
    pow(
      static_cast<double>( n + 1 ),
      static_cast<double>( n - 1 )
    )
    ==
    rank_vector.size()
  );

  // Compare branchings in heap and ranked vector.  Allow for the possibility
  // that two branchings might have the same weight (hence the second loop).

  bool correct = true;

  bool found;

  std::set<size_t> check;

  for( size_t i = 0; i < rank_vector.size(); i++ )
  {
    check.insert( i );
  }

  while( !branching_heap.empty() )
  {

    for(
      std::set<size_t>::iterator it = check.begin();
      it != check.end();
      it++
    )
    {

      found = true;

      if( rank_vector[*it].weight != branching_heap.top().weight )
      {
        found = false;
        break;
      }

      BOOST_FOREACH( Edge e, branching_heap.top().v_edges )
      {
        if(
          rank_vector[*it].v_edges.find( e ) == rank_vector[*it].v_edges.end()
        )
        {
          found = false;
        }
      }

      if( found )
      {
        check.erase( it );
        break;
      }
    }

    if( !found )
    {
      correct = false;
      std::cout << "Error: " << std::endl;
      BOOST_FOREACH( Edge e, branching_heap.top().v_edges )
      {
        std::cout << " (" << index_map[source( e, g )] << "," <<
                     index_map[target( e, g )] << ") ";
      }
      std::cout << " Weight: " << branching_heap.top().weight << std::endl;
      std::cout << std::endl;
    }
    branching_heap.pop();
  }

  if( correct )
  {
    return EXIT_SUCCESS;
  }
  else
  {
    return EXIT_FAILURE;
  }
}
