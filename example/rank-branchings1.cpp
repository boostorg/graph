// Copyright 2015 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <iostream>
#include <fstream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/rank_spanning_branchings.hpp>

template<class Graph>
struct print_branching
{

  const Graph& m_g;

  typedef
    typename boost::property_map<Graph, boost::edge_weight_t>::const_type
    WeightMap;

  WeightMap w;
  typename boost::property_traits<WeightMap>::value_type weight;

  print_branching( const Graph& g ) : m_g( g ) {} 

  template<class EdgeIterator>
  bool operator()( std::pair<EdgeIterator, EdgeIterator> p )
  {

    std::cout << "Branching:";

    weight = 0.;

    while( p.first != p.second )
    {

      std::cout << " (" << boost::source( *p.first, m_g ) << "," <<
        boost::target( *p.first, m_g ) << ")";

      weight += get( w, *p.first++ );

    }

    std::cout << std::endl << "  Weight: " << weight << std::endl << std::endl;

    return true;

  }

};
   
int
main()
{
  typedef boost::adjacency_list < boost::vecS, boost::vecS, boost::directedS,
    boost::no_property, boost::property < boost::edge_weight_t, int > > Graph;
  typedef boost::graph_traits < Graph >::edge_descriptor Edge;
  typedef boost::graph_traits < Graph >::vertex_descriptor Vertex;
  typedef std::pair<int, int> E;

  const int num_nodes = 4;
  E edge_array[] = { E(0, 1), E(0, 2), E(0, 3), E(1, 2), E(2, 1), E(2, 3),
    E(3, 2), E(1, 3), E(3, 1)
  };
  int weights[] = { 5, 1, 1, 11, 10, 5, 8, 4, 9 };

  std::size_t num_edges = sizeof(edge_array) / sizeof(E);
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  Graph g(num_nodes);
  boost::property_map<Graph, boost::edge_weight_t>::type weightmap =
    get(edge_weight, g);
  for (std::size_t j = 0; j < num_edges; ++j) {
    Edge e; bool inserted;
    boost::tie(e, inserted) = add_edge(edge_array[j].first, edge_array[j].second, g);
    weightmap[e] = weights[j];
  }
#else
  Graph g(edge_array, edge_array + num_edges, weights, num_nodes);
#endif

  // Rank branchings in descending order of weight (use default
  // weight comparison).

  std::cout << std::endl;

  std::cout << "Spanning branchings in descending order of weight:"
            << std::endl << std::endl;

  boost::rank_spanning_branchings(
    g,
    print_branching<Graph>( g )
  );

  std::cout << std::endl;

  // Rank branchings in ascending order of weight (use supplied
  // weight comparison).

  std::cout << "Spanning branchings in ascending order of weight:"
            << std::endl << std::endl;

  boost::rank_spanning_branchings(
    g,
    print_branching<Graph>( g ),
    boost::edge_compare( std::greater<int>() )
  );

  return EXIT_SUCCESS;
}
