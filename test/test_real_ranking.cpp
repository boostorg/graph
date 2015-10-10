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
// arc is assigned a random real weight between -10 and 10.  The test
// then constructs all potential branchings and discards those
// that have indegree for any vertex greater than one or contain
// a cycle.  The valid branchings are then stored in a priority
// queue with largest weight branching at the top.  The test then
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
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <boost/graph/rank_spanning_branchings.hpp>

using namespace boost;

template<typename Edge>
struct Branching
{
  double                         weight;
  boost::unordered_set<Edge>  v_edges;
  Branching( double w, const boost::unordered_set<Edge>& v_e ) :
    weight( w ), v_edges( v_e ){}
  bool operator<( Branching const & rhs ) const
  { return weight < rhs.weight; }
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
  size_t n;

  set_rank_vector(
    const Graph& g,
    std::vector<Branching<Edge> >& rv
  ) : m_g( g ), rank_vector( rv )
  {}

  template<class EdgeIterator>
  bool operator()( std::pair<EdgeIterator, EdgeIterator> p )
  {

    w = get( edge_weight, m_g );

    weight = 0;

    EdgeIterator ei, ei_end;

    for( boost::tie( ei, ei_end ) = p; ei != ei_end; ei++ )
    {
      weight += get( w, *ei );

      branching.insert( *ei );
    }

    rank_vector.push_back( Branching<Edge>( weight, branching ) ); 

    return true;

  }

};

template<class Graph, class Edge>
void
set_branching(
  Graph& g,
  const std::vector<Edge>& v_edges,
  const std::vector<size_t>& combination,
  heap::fibonacci_heap<Branching<Edge> >& branching_heap
)
{
  disjoint_sets_with_storage<> W( num_vertices( g ) );
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  unordered_set<Vertex> in_vertex_set;
  typename property_map<Graph, edge_weight_t>::type weightmap =
    get(edge_weight, g);
  boost::unordered_set<Edge> branching_edges;

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

  double weight = 0;
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
  if (k == 0) {
    set_branching(g, v_edges, combination, branching_heap);
    return;
  }
  for (size_t i = offset; i <= indices.size() - k; ++i) {
    combination.push_back(indices[i]);
    loop(i+1, k-1, g, v_edges, indices, combination, branching_heap);
    combination.pop_back();
  }
}

int main( int argc, char **argv )
{

  typedef adjacency_list < vecS, vecS, directedS,
     no_property, property < edge_weight_t, double > > Graph;
  typedef graph_traits <Graph>::edge_descriptor Edge;
  heap::fibonacci_heap<Branching<Edge> > branching_heap;
  typedef random::mt19937 base_generator_type;

  std::vector<size_t> indices;
  std::vector<size_t> combination;
  std::vector<Edge> v_edges;

  base_generator_type gen;
  gen.seed( (unsigned int) time( NULL ) );

  boost::random::uniform_real_distribution<> tdist( -10, 10 );

  boost::variate_generator<
    base_generator_type&, boost::random::uniform_real_distribution<>
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

  size_t n = boost::lexical_cast<size_t>( argv[1] );

  for( size_t i = 0; i < n; i++ )
  {
    for( size_t j = 0; j < n; j++ )
    {
      if( i != j ) add_edge( i, j, rvt(), g );
    }
    add_edge( n, i, 0, g );
  }

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
    set_rank_vector<Graph, Edge>( g, rank_vector )
  );

  // Check that number of branchings found is correct.

  if(
    pow(
      static_cast<double>( n + 1 ),
      static_cast<double>( n - 1 )
    )
    !=
    rank_vector.size()
  )
  {
    std::cerr << "Incorrect number of branchings." << std::endl;
    return EXIT_FAILURE;
  }

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
      std::cerr << "Error: " << std::endl;
      BOOST_FOREACH( Edge e, branching_heap.top().v_edges )
      {
        std::cerr << " (" << source( e, g ) << "," << target( e, g ) << ") ";
      }
      std::cerr << " Weight: " << branching_heap.top().weight << std::endl;
      std::cerr << std::endl;
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
