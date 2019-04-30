//=======================================================================
// Copyright (c) 2005 Aaron Windsor
//
// Distributed under the Boost Software License, Version 1.0. 
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//
//=======================================================================
#include <cstdlib>
#include <iostream>
#include <boost/property_map/vector_property_map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/random.hpp>
#include <ctime>
#include <random>
#include <boost/core/lightweight_test.hpp>

#include <boost/graph/max_cardinality_matching.hpp>

using namespace boost;

typedef adjacency_list<vecS, 
                       vecS, 
                       undirectedS, 
                       property<vertex_index_t, int> >  undirected_graph;

typedef property_map<undirected_graph,vertex_index_t>::type vertex_index_map_t;
typedef vector_property_map<graph_traits<undirected_graph>::vertex_descriptor, vertex_index_map_t > mate_t;
typedef graph_traits<undirected_graph>::vertex_iterator vertex_iterator_t;
typedef graph_traits<undirected_graph>::vertex_descriptor vertex_descriptor_t;
typedef graph_traits<undirected_graph>::vertices_size_type v_size_t;


int main(int argc, char** argv)
{  
  if (argc < 3)
    {
      BOOST_TEST(false);
      return boost::report_errors();
    }
  
  int n = atoi(argv[1]);
  int m = atoi(argv[2]);

  undirected_graph g(n);
  

  std::mt19937 generator(static_cast<unsigned int>(std::time(0)));
  boost::uniform_int<> distribution(0, n-1);
  boost::variate_generator<std::mt19937&, boost::uniform_int<> > rand_num(generator, distribution);

  int num_edges = 0;
  bool success;

  while (num_edges < m)
    {
      vertex_descriptor_t u = random_vertex(g,rand_num);
      vertex_descriptor_t v = random_vertex(g,rand_num);
      if (u != v)
        {
          if (!edge(u,v,g).second)
            boost::tie(tuples::ignore, success) = add_edge(u, v, g);
          else
            success = false;

          if (success)
            num_edges++;
        }
    }

  mate_t mate(n);
  bool random_graph_result = checked_edmonds_maximum_cardinality_matching(g,mate);  
 
  BOOST_TEST(random_graph_result);

  //Now remove an edge from the random_mate matching.
  vertex_iterator_t vi, vi_end;
  for(boost::tie(vi,vi_end) = vertices(g); vi != vi_end; ++vi)
    if (mate[*vi] != graph_traits<undirected_graph>::null_vertex())
      break;
  
  mate[mate[*vi]] = graph_traits<undirected_graph>::null_vertex();
  mate[*vi] = graph_traits<undirected_graph>::null_vertex();
  
  //...and run the matching verifier - it should tell us that the matching isn't
  //a maximum matching.
  bool modified_random_verification_result =
    maximum_cardinality_matching_verifier<undirected_graph,mate_t,vertex_index_map_t>::verify_matching(g,mate,get(vertex_index,g));
  
  BOOST_TEST(!modified_random_verification_result);

  //find a greedy matching on the graph
  mate_t greedy_mate(n);
  greedy_matching<undirected_graph, mate_t>::find_matching(g,greedy_mate);

  BOOST_TEST(!(matching_size(g,mate) > matching_size(g,greedy_mate) &&
      maximum_cardinality_matching_verifier<undirected_graph,mate_t,vertex_index_map_t>::verify_matching(g,greedy_mate,get(vertex_index,g))));

  return boost::report_errors();
}

