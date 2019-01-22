//            Copyright Daniel Trebbien 2010.
// Distributed under the Boost Software License, Version 1.0.
//   (See accompanying file LICENSE_1_0.txt or the copy at
//         http://www.boost.org/LICENSE_1_0.txt)

#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/one_bit_color_map.hpp>
#include <boost/graph/stoer_wagner_min_cut.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/typeof/typeof.hpp>

struct edge_t
{
  unsigned long first;
  unsigned long second;
};

// A graphic of the min-cut is available at <http://www.boost.org/doc/libs/release/libs/graph/doc/stoer_wagner_imgs/stoer_wagner.cpp.gif>
int main()
{ 
  using undirected_graph = boost::adjacency_list<boost::vecS, boost::vecS, boost::undirectedS,
    boost::no_property, boost::property<boost::edge_weight_t, int>>;
  using weight_map_type = boost::property_map<undirected_graph, boost::edge_weight_t>::type;
  using weight_type = boost::property_traits<weight_map_type>::value_type;
  
  // define the 16 edges of the graph. {3, 4} means an undirected edge between vertices 3 and 4.
  edge_t edges[] = {{3, 4}, {3, 6}, {3, 5}, {0, 4}, {0, 1}, {0, 6}, {0, 7},
    {0, 5}, {0, 2}, {4, 1}, {1, 6}, {1, 5}, {6, 7}, {7, 5}, {5, 2}, {3, 4}};
  
  // for each of the 16 edges, define the associated edge weight. ws[i] is the weight for the edge
  // that is described by edges[i].
  weight_type ws[] = {0, 3, 1, 3, 1, 2, 6, 1, 8, 1, 1, 80, 2, 1, 1, 4};
  
  // construct the graph object. 8 is the number of vertices, which are numbered from 0
  // through 7, and 16 is the number of edges.
  undirected_graph g(std::begin(edges), std::end(edges), ws, 8, 16);
  
  // define a property map, `parities`, that will store a boolean value for each vertex.
  // Vertices that have the same parity after `stoer_wagner_min_cut` runs are on the same side of the min-cut.
  BOOST_AUTO(parities, boost::make_one_bit_color_map(num_vertices(g), get(boost::vertex_index, g)));
  
  // run the Stoer-Wagner algorithm to obtain the min-cut weight. `parities` is also filled in.
  auto w = boost::stoer_wagner_min_cut(g, get(boost::edge_weight, g), boost::parity_map(parities));
  
  std::cout << "The min-cut weight of G is " << w << ".\n" << std::endl;
  assert(w == 7);
  
  std::cout << "One set of vertices consists of:" << std::endl;
  size_t i;
  for (i = 0; i < num_vertices(g); ++i) {
    if (get(parities, i))
      std::cout << i << std::endl;
  }
  std::cout << std::endl;
  
  std::cout << "The other set of vertices consists of:" << std::endl;
  for (i = 0; i < num_vertices(g); ++i) {
    if (!get(parities, i))
      std::cout << i << std::endl;
  }
  std::cout << std::endl;
  
  return EXIT_SUCCESS;
}
