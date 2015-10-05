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
// functor (in this case, my_function) and applies the functor to each
// edge in the branching.  If the functor returns false for any edge,
// rank_spanning_branchings will return after processing all edges in the
// current branching.

using namespace boost;

template<class Graph>
class my_function
{

  public:

    my_function(
      const Graph& g, double& _max_weight, double _cut
    ) : m_g( g ), max_weight( _max_weight ), cut( _cut )
    {
      w = boost::get( boost::edge_weight, m_g );
      weight = 0;
      n = 1;
      name_map = get( vertex_name, m_g );
      b_string = "";
    }

    template<class Edge>
    bool operator()( const Edge& e )
    {

      weight += get( w, e );

      std::stringstream ss;
      ss << "(" << name_map[source( e, m_g )] << ", " <<
            name_map[target( e, m_g )] << ") ";

      b_string += ss.str();

      if( ++n == num_vertices( m_g ) )  // Last edge in branching.
      {

        if( max_weight == -std::numeric_limits<double>::infinity() )
        {
          max_weight = weight;
        }

        d_diff = weight - max_weight;

        if( d_diff < cut ){
          std::cout << std::endl;
          return false;
        }  // Stop before output.

        std::cout << "\nBranching: " << b_string << std::endl;

        std::cout << "  Weight = " << weight << std::endl;

        std::cout << "  Weight - Max Weight = " << d_diff << std::endl;

        return true;

      }

      return true;

    }

  private:
    const Graph& m_g;
    double& max_weight;
    double cut;
    typedef
      typename boost::property_map<Graph, boost::edge_weight_t>::const_type
      WeightMap;
    WeightMap w;
    typename boost::property_traits<WeightMap>::value_type weight;
    size_t n;
    typename property_map < Graph, vertex_name_t >::const_type name_map;
    std::string b_string;
    double d_diff;

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
      s_map[str[0]], s_map[str[1]], lexical_cast<double>( str[2] ), g
    );

  }

}

int main( int argc, char **argv )
{

  typedef adjacency_list <
      listS, listS, directedS,
      property <
        vertex_index_t, size_t,
        property< vertex_name_t, std::string >
      >,
      property < edge_weight_t, double >
    > Graph;
  std::ifstream input_file;
  double max_weight = -std::numeric_limits<double>::infinity();

  if( argc != 3 )
  {
    std::cerr << std::endl;
    std::cerr << "  Usage: " << argv[0] <<
                 " file  cut" << std::endl << std::endl;
    std::cerr << "    file = input file" << std::endl;
    std::cerr << "    cut = branching weight cut" << std::endl << std::endl;
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

  rank_spanning_branchings(
    g,
    my_function<Graph>(
      g,
      max_weight,
      boost::lexical_cast<double>( argv[2] )
    )
  );

}
