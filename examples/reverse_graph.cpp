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

#include <boost/config.hpp>

#include <algorithm>
#include <vector>
#include <utility>
#include <iostream>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/reverse_graph.hpp>
#include <boost/graph/graph_utility.hpp>

int
main()
{
  typedef boost::adjacency_list< 
    boost::vecS, boost::vecS, boost::bidirectionalS
  > Graph;
  
  Graph G(5);
  boost::add_edge(0, 2, G);
  boost::add_edge(1, 1, G);
  boost::add_edge(1, 3, G);
  boost::add_edge(1, 4, G);
  boost::add_edge(2, 1, G);
  boost::add_edge(2, 3, G);
  boost::add_edge(2, 4, G);
  boost::add_edge(3, 1, G);
  boost::add_edge(3, 4, G);
  boost::add_edge(4, 0, G);
  boost::add_edge(4, 1, G);

  std::cout << "original graph:" << std::endl;
  boost::print_graph(G, boost::get(boost::vertex_index, G));

  std::cout << std::endl << "reversed graph:" << std::endl;
  boost::print_graph(boost::make_reverse_graph(G), 
                     boost::get(boost::vertex_index, G));


  return 0;
}
