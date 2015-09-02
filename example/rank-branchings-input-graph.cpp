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

    ./rank-branchings-input-graph input.txt -10

*/

#include <iostream>
#include <fstream>
#include <vector>
#include <iostream>
#include <algorithm>

#include <boost/graph/adjacency_list.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_real_distribution.hpp>
#include <boost/random/variate_generator.hpp>

#include <boost/graph/iteration_macros.hpp>

#include <boost/graph/rank_spanning_branchings.hpp>

using namespace boost;

class my_function
{

  public:

    my_function( double _cut) : cut( _cut ) { i_flag = 0; max_weight = 0; }

    template<class Graph, class Edge>
    bool operator()( const Graph& G, const boost::unordered_set<Edge>& b )
    {

       double d_diff;

       typedef
         typename boost::property_map<Graph, boost::edge_weight_t>::const_type
         WeightMap;

       typename property_map < Graph, vertex_name_t >::const_type
         name_map = get( vertex_name, G );

       WeightMap w = boost::get( boost::edge_weight, G );

       typename boost::property_traits<WeightMap>::value_type weight = 0;

       BOOST_FOREACH( const Edge& e, b )
       {
         weight += get( w, e );
       }

       if( i_flag == 0 )
       {
         max_weight = weight;
         i_flag = 1;
       }

       d_diff = weight - max_weight;

       if( d_diff < cut ) return false;

       std::cout << "Branching: ";

       BOOST_FOREACH( const Edge& e, b )
       {
         std::cout << "(" << name_map[source( e, G )] << "," <<
                      name_map[target( e, G )] << ") ";
       }
       std::cout << std::endl;

       std::cout << "  Weight = " << weight << std::endl;

       std::cout << "  Weight - Max Weight = " << d_diff << std::endl
                 << std::endl;

       return true;

    }

  private:
    double cut;
    double max_weight;
    int i_flag;

};
   
template <typename Graph>
void
read_graph_file(
  std::istream & graph_in,
  Graph & g
)
{
  typedef typename graph_traits<Graph>::vertices_size_type size_type;
  typedef typename property_map < Graph, vertex_index_t >::type index_map_t;
  typedef typename property_map < Graph, vertex_name_t >::type name_map_t;
  size_type n_vertices;
  typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
  Vertex u;
  typename property_traits <name_map_t>::value_type name;
  std::map<std::string, Vertex> s_map;

  index_map_t index_map = get( vertex_index, g );
  name_map_t name_map = get( vertex_name, g );

  graph_in >> n_vertices;
  for (size_type i = 0; i < n_vertices; ++i)
  {
    u = add_vertex( g );
    index_map[u] = i;
    graph_in >> name;
    put( name_map, u, name );
    s_map[name] = u;
  }

  std::string src, targ;
  double weight;

  while( graph_in >> src >> targ >> weight )
  {
    add_edge( s_map[src], s_map[targ], weight, g );
  }

}

int main( int argc, char **argv )
{

  std::ifstream input_file;
  typedef adjacency_list <
      listS, listS, directedS,
      property <
        vertex_index_t, size_t,
        property< vertex_name_t, std::string >
      >,
      property < edge_weight_t, double >
    > Graph;

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

  std::ifstream file_in( argv[1] );

  read_graph_file( file_in, g );

  rank_spanning_branchings(
    g,
    std::numeric_limits<size_t>::max(),
    my_function( boost::lexical_cast<double>( argv[2] ) )
  );

}
