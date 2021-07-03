//=======================================================================
// Copyright 2015 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

/*
  Example execution:

    ./rank-branchings2 branching_input.txt -10

*/

#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/rank_spanning_branchings.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

// When rank_spanning_branchings finds a branching, it calls the user-defined 
// functor (in this case, my_function).  The functor takes as input
// a filtered view of the parent graph that gives the branching.  If the
// functor returns false, rank_spanning_branchings will stop ranking the
// branchings and will return.  If the functor returns true,
// rank_spanning_branchings will continue to find the next branching.

using namespace boost;

template<class Graph>
class my_function
{

  public:

    typedef
      typename property_map<Graph, edge_weight_t>::const_type WeightMap;

    typedef
      typename property_traits<WeightMap>::value_type weight_t;

    my_function( weight_t& _max_weight, weight_t _cut ) :
      max_weight( _max_weight ), cut( _cut )
    {}

    template<class BranchingGraph>
    bool operator()( BranchingGraph& bg )
    {

      WeightMap w;

      typename property_map<BranchingGraph, vertex_name_t>::const_type name_map;

      weight_t weight = 0;

      b_string = "";

      BGL_FORALL_EDGES_T( e, bg, BranchingGraph )
      {

        weight += get( w, e );

        std::stringstream ss;
        ss << "(" << name_map[source( e, bg )] << ", " <<
              name_map[target( e, bg )] << ") ";

        b_string += ss.str();
      }

      if( max_weight == -std::numeric_limits<weight_t>::infinity() )
      {
        max_weight = weight;
      }

      d_diff = weight - max_weight;

      if( d_diff < cut )
      {
        std::cout << std::endl;
        return false;
      }  // Stop before output.

      std::cout << "Branching: " << b_string << std::endl;

      std::cout << "  Weight = " << weight << std::endl;

      std::cout << "  Weight - Max Weight = " << d_diff << std::endl;

      std::cout << std::endl;

      return true;

    }

  private:
    weight_t& max_weight;
    weight_t cut;
    std::string b_string;
    weight_t d_diff;

};
   
// Check for vertex from input file and add if not already present.

template <typename Graph, typename Vertex>
void
check_vertex( Graph& g, std::map<std::string, Vertex>& s_map, std::string str )
{

  typedef typename property_map < Graph, vertex_index_t >::type index_map_t;
  typedef typename property_map < Graph, vertex_name_t >::type name_map_t;

  index_map_t index_map = get( vertex_index, g );
  name_map_t name_map = get( vertex_name, g );

  if( s_map.find( str ) == s_map.end() )
  {
    Vertex u = add_vertex( g );
    index_map[u] = s_map.size();
    put( name_map, u, str );
    s_map[str] = u;
  }

}

// Read in the graph from the file.  Each line in the input file should be
// a comma-delimited triplet (source, target, weight).

template <typename Graph>
void
read_graph_file(
  std::istream & graph_in,
  Graph & g
)
{

  typedef typename property_map<Graph, edge_weight_t>::const_type WeightMap;

  typedef typename property_traits<WeightMap>::value_type weight_t;

  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  std::map<std::string, Vertex> s_map;
  std::string text;

  char_separator<char> sep(",");

  while( std::getline( graph_in, text ) )
  {

    std::vector<std::string> str;
    tokenizer<char_separator<char> > tokens( text, sep );

    BOOST_FOREACH( const std::string& name, tokens )
    {
      std::string s = name;
      algorithm::trim( s );
      str.push_back( s );
    }

    check_vertex( g, s_map, str[0] );  // Check source;
    check_vertex( g, s_map, str[1] );  // Check target;

    add_edge(
      s_map[str[0]], s_map[str[1]], lexical_cast<weight_t>( str[2] ), g
    );

  }

}

int main( int argc, char **argv )
{

  typedef int my_type;

  typedef adjacency_list <
      listS, listS, directedS,
      property <
        vertex_index_t, size_t,
        property< vertex_name_t, std::string >
      >,
      property < edge_weight_t, my_type>
    > Graph;
  std::ifstream input_file;
  my_type max_weight = -std::numeric_limits<my_type>::infinity();

  if( argc != 3 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] <<
                 " file  cut" << std::endl << std::endl;
    std::cerr << "    file = input file" << std::endl;
    std::cerr << "    cut = branching weight cut" << std::endl << std::endl;
    std::cerr << "  Example usage: " << argv[0] <<
                 " branching_input.txt -10" << std::endl << std::endl;;
    return EXIT_FAILURE;
  }

  Graph g;

  input_file.open( argv[1] );

  if( !input_file.is_open() )
  {
    std::cerr << "Invalid input file." << std::endl;
    return EXIT_FAILURE;
  }

  std::ifstream file_in( argv[1] );

  read_graph_file( file_in, g );

  std::cout << std::endl;

  rank_spanning_branchings(
    g,
    my_function<Graph>(
      max_weight,
      lexical_cast<my_type>( argv[2] )
    )
  );

  return EXIT_SUCCESS;

}
