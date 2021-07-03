//=======================================================================
// Copyright 2015-2018 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

// This test first creates a complete graph or multigraph with n vertices.
// Each arc is assigned a random integer, double, or defined class
// weight.  A root vertex is added and arcs with zero weight
// are added from the root vertex to each of the other vertices.  The test
// then constructs all potential branchings and discards those
// that have indegree for any vertex greater than one or contain
// a cycle.  The valid branchings are then stored in a priority
// queue.  The test then
// uses the rank_spanning_branchings.hpp routine to construct all
// branchings, in order, and places them in a vector.  The vector
// and the priority queue are then compared, allowing for the possibility
// of equal weight branchings.

#include <iostream>
#include <vector>
#include <complex>

#include <boost/graph/adjacency_list.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int_distribution.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/assert.hpp>

#include <boost/graph/rank_spanning_branchings.hpp>

using namespace boost;

typedef random::mt19937 base_generator_type;

template<typename T, typename U>
class my_type
{
  public:

    my_type(){}
    my_type( T _t, U _u ) : t( _t ), u( _u ) {}

    my_type& operator+=( const my_type& rhs )
    {
      this->t += rhs.t;
      this->u += rhs.u;
      return *this;
    }

    friend my_type operator+( my_type lhs, const my_type& rhs )
    {
      lhs += rhs;
      return lhs;
    }

    my_type& operator-=( const my_type& rhs )
    {
      this->t -= rhs.t;
      this->u -= rhs.u;
      return *this;
    }

    friend my_type operator-( my_type lhs, const my_type& rhs )
    {
      lhs -= rhs;
      return lhs;
    }

    bool operator!=( const my_type& rhs )
    {
      return !( ( this->t == rhs.t ) && ( this->u == rhs.u ) );
    }

    bool operator<( const my_type& rhs ) const
    {
      if( this->t == rhs.t )
       return this->u < rhs.u;
      else
       return this->t < rhs.t;
    }

    friend std::ostream& operator<<( std::ostream& os, const my_type& p )
    {
      os << "(" << p.t << ", " << p.u << ")";
      return os;
    }

  private:
    T t;
    U u;

};

template<class Graph, class Edge>
struct Branching
{
  typedef
    typename
    property_traits<
      typename property_map<Graph, edge_weight_t>::const_type
    >::value_type weight_t;
  weight_t weight;
  std::set<Edge>  v_edges;
  Branching( weight_t & w, const std::set<Edge>& v_e) :
    weight( w ), v_edges( v_e ){}
};

template<class Graph, class Edge, class Compare>
struct compare_branchings
{
  Compare comp;
  compare_branchings( Compare _comp ) : comp( _comp ){}
  bool operator()(
    const Branching<Graph,Edge>& b1,
    const Branching<Graph,Edge>& b2 ) const
  {
    return comp( b1.weight, b2.weight );
  }
};


template<class Graph, class Edge>
struct set_rank_vector
{

  std::vector<Branching<Graph,Edge> >& rank_vector;
  set_rank_vector(
    std::vector<Branching<Graph,Edge> >& rv
  ) : rank_vector( rv ) {}

  template<class BranchingGraph>
  bool operator()( BranchingGraph& bg )
  {

    std::set<Edge> branching;

    typedef
      typename
        property_map<BranchingGraph, edge_weight_t>::const_type weight_map_t;
    weight_map_t w;
    typedef typename property_traits<weight_map_t>::value_type weight_t;

    scoped_ptr<weight_t> weight;
    weight.reset();

    BGL_FORALL_EDGES_T( e, bg, BranchingGraph )
    {
      if( !weight )
      {
        weight.reset( new weight_t );
        *weight = get( w, e );
      }
      else
      {
        *weight += get( w, e );
      }
      branching.insert( e );
    }

    rank_vector.push_back(Branching<Graph,Edge>( *weight, branching ) ); 

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
  std::vector<Branching<Graph,Edge> >& branchings
)
{
  disjoint_sets<Rank, Parent> W( rank, parent );
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  std::set<Vertex> in_vertex_set;
  typedef typename property_map<Graph, edge_weight_t>::const_type weight_map_t;
  typedef typename property_traits<weight_map_t>::value_type weight_t;
  std::set<Edge> branching_edges;

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
  
  scoped_ptr<weight_t> weight;
  weight.reset();

  BOOST_FOREACH( size_t i, combination )
  {
    Edge e = v_edges[i];
    if( !weight )
    {
      weight.reset( new weight_t );
      *weight = get( edge_weight, g, e );
    }
    else
    {
      *weight += get( edge_weight, g, e );
    }
    branching_edges.insert( e );
  }
  branchings.push_back( Branching<Graph,Edge>( *weight, branching_edges ) );
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
  std::vector<Branching<Graph,Edge> >& branchings
)
{

  typedef typename graph_traits<Graph>::vertices_size_type size_type;
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;

  size_type n = num_vertices( g );
  std::vector<size_type> rank_map( n );
  std::vector<vertex_t> pred_map( n );

  if( k == 0 )
  {
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
      branchings
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
      branchings
    );
    combination.pop_back();
  }
}

template<class Graph, class Edge, class Compare>
bool
compare_heap_and_vector(
  Graph& g,
  heap::fibonacci_heap<
    Branching<Graph,Edge>,
    heap::compare<Compare>
  >& branching_heap,
  std::vector<Branching<Graph,Edge> >& rank_vector
)
{

  // Compare branchings in heap and ranked vector.  Allow for the possibility
  // that two branchings might have the same weight (hence the second loop).

  typename property_map<Graph, vertex_index_t>::type index_map =
    get( vertex_index, g );

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
      std::cerr << "Error: " << std::endl;
      BOOST_FOREACH( Edge e, branching_heap.top().v_edges )
      {
        std::cerr <<
        " (" << index_map[source( e, g )] <<
        "," << index_map[target( e, g )] << ") ";
      }
      std::cerr << " Weight: " << branching_heap.top().weight << std::endl;
        std::cerr << std::endl;
      return false;
    }
    branching_heap.pop();
  }

  return true;

}

template<class Compare>
void test_int_graph( base_generator_type& gen, size_t n, Compare comp )
{

  typedef adjacency_list < listS, setS, directedS,
     property < vertex_index_t, size_t >, property < edge_weight_t, int > >
     Graph;
  typedef property_map < Graph, vertex_index_t >::type index_map_t;
  typedef graph_traits <Graph>::edge_descriptor Edge;

  std::vector<size_t> indices;
  std::vector<size_t> combination;
  std::vector<Edge> v_edges;
  std::vector<Branching<Graph,Edge> > branchings, rank_vector;

  typedef
    heap::fibonacci_heap<
      Branching<Graph,Edge>,
      heap::compare<compare_branchings<Graph,Edge,Compare> >
    > branching_heap_t;

  branching_heap_t branching_heap( comp );

  random::uniform_int_distribution<> tdist( -10, 10 );

  variate_generator<
    base_generator_type&, random::uniform_int_distribution<>
  > rvt( gen, tdist );

  Graph g;

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
    branchings
  );

  for( size_t i = 0; i < branchings.size(); i++ )
  {
    branching_heap.push( branchings[i] );
  }

  rank_spanning_branchings(
    g,
    set_rank_vector<Graph,Edge>( rank_vector ),
    distance_compare( comp )
  );

  // Check that number of branchings found is correct.

  assert(
    pow(
      static_cast<double>( n + 1 ),
      static_cast<double>( n - 1 )
    )
    ==
    rank_vector.size()
  );

  assert( branching_heap.size() == rank_vector.size() );

  // Compare branchings found by both methods.

  if( !compare_heap_and_vector( g, branching_heap, rank_vector ) )
  {
    exit( EXIT_FAILURE );
  }

}

template<class Compare>
void test_real_graph( base_generator_type& gen, size_t n, Compare comp )
{

  typedef adjacency_list < vecS, vecS, directedS,
     no_property, property < edge_weight_t, double > > Graph;
  typedef graph_traits <Graph>::edge_descriptor Edge;

  std::vector<size_t> indices;
  std::vector<size_t> combination;
  std::vector<Edge> v_edges;
  std::vector<Branching<Graph,Edge> > branchings, rank_vector;

  typedef
    heap::fibonacci_heap<
      Branching<Graph,Edge>,
      heap::compare<compare_branchings<Graph,Edge,Compare> >
    > branching_heap_t;

  branching_heap_t branching_heap( comp );

  boost::random::uniform_real_distribution<> tdist( -10, 10 );

  boost::variate_generator<
    base_generator_type&, boost::random::uniform_real_distribution<>
  > rvt( gen, tdist );

  Graph g;

  for( size_t i = 0; i < n; i++ )
  {
    for( size_t j = 0; j < n; j++ )
    {
      if( i != j )
      {
        add_edge( i, j, rvt(), g );
        add_edge( i, j, rvt(), g );    // A parallel edge.
      }
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
    branchings
  );

  for( size_t i = 0; i < branchings.size(); i++ )
  {
    branching_heap.push( branchings[i] );
  }

  // Call explicitly with named parameters to test.

  rank_spanning_branchings(
    g,
    set_rank_vector<Graph, Edge>( rank_vector ),
    distance_compare( comp ).
    weight_map( get( edge_weight, g ) ).
    vertex_index_map( get( vertex_index, g ) )
  );

  // Check that number of branchings found is correct.  The formula
  // accounts for the parallel edges.

  assert(
    pow(
      static_cast<double>( 2*n + 1 ),
      static_cast<double>( n - 1 )
    )
    ==
    rank_vector.size()
  );

  assert( branchings.size() == rank_vector.size() );

  // Compare branchings found by both methods.

  if( !compare_heap_and_vector( g, branching_heap, rank_vector ) )
  {
    exit( EXIT_FAILURE );
  }

}

template<class Compare>
void test_class_graph( base_generator_type& gen, size_t n, Compare comp )
{

  typedef adjacency_list < vecS, vecS, directedS,
     no_property, property < edge_weight_t, my_type<double, double > >
  > Graph;
  typedef graph_traits <Graph>::edge_descriptor Edge;

  std::vector<size_t> indices;
  std::vector<size_t> combination;
  std::vector<Edge> v_edges;
  std::vector<Branching<Graph,Edge> > branchings, rank_vector;

  typedef
    heap::fibonacci_heap<
      Branching<Graph,Edge>,
      heap::compare<compare_branchings<Graph,Edge,Compare> >
    > branching_heap_t;

  branching_heap_t branching_heap( comp );

  boost::random::uniform_real_distribution<> tdist( -10, 10 );

  boost::variate_generator<
    base_generator_type&, boost::random::uniform_real_distribution<>
  > rvt( gen, tdist );

  Graph g;

  for( size_t i = 0; i < n; i++ )
  {
    for( size_t j = 0; j < n; j++ )
    {
      if( i != j )
      {
        add_edge( i, j, my_type<double, double>( rvt(), rvt() ), g );
      }
    }
    add_edge( n, i, my_type<double, double>( 0, 0 ), g );
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
    branchings
  );

  for( size_t i = 0; i < branchings.size(); i++ )
  {
    branching_heap.push( branchings[i] );
  }

  // Call explicitly with named parameters to test.

  rank_spanning_branchings(
    g,
    set_rank_vector<Graph, Edge>( rank_vector ),
    distance_compare( comp ).
    weight_map( get( edge_weight, g ) ).
    vertex_index_map( get( vertex_index, g ) )
  );

  // Check that number of branchings found is correct.

  assert(
    pow(
      static_cast<double>( n + 1 ),
      static_cast<double>( n - 1 )
    )
    ==
    rank_vector.size()
  );

  assert( branchings.size() == rank_vector.size() );

  // Compare branchings found by both methods.

  if( !compare_heap_and_vector( g, branching_heap, rank_vector ) )
  {
    exit( EXIT_FAILURE );
  }

}

int main( int argc, char **argv )
{

  if( argc != 3 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] << " n" << std::endl << std::endl;
    std::cerr << "    n = number of vertices in complete graph" <<
                 std::endl << std::endl;
    std::cerr << "    k = number of trials" << std::endl << std::endl;
    return EXIT_FAILURE;
  }

  base_generator_type gen;
  gen.seed( (unsigned int) time( NULL ) );

  for( size_t i = 0; i < lexical_cast<size_t>( argv[2] ); i++ )
  {
    test_int_graph(
      gen, lexical_cast<size_t>( argv[1] ) + 1, std::greater<int>()
    );
    test_real_graph(
      gen, lexical_cast<size_t>( argv[1] ), std::less<double>()
    );
    test_class_graph(
      gen,
      lexical_cast<size_t>( argv[1] ) + 1,
      std::less<my_type<double, double> >()
    );
  }

  std::cout << "tests passed" << std::endl;

  return EXIT_SUCCESS;

}

