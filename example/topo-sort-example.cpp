//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University,
// Bloomington, IN 47405.
//
// Permission to modify the code and to distribute the code is
// granted, provided the text of this NOTICE is retained, a notice if
// the code was modified is included with the above COPYRIGHT NOTICE
// and with the COPYRIGHT NOTICE in the LICENSE file, and that the
// LICENSE file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
#line 132 "topological-sort.w"

#include <boost/config.hpp>
#include <fstream>
#include <iostream>
#include <list>
#include <algorithm>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/graphviz.hpp>
#include <iterator>
#include <utility>

struct format_writer
{
  void operator() (std::ostream & out) const
  {
    out << "node[shape=\"circle\"]\nsize=\"3,3\"\nratio=\"fill\"\n";
  }
};

int
main()
{
  using namespace boost;

#line 100 "topological-sort.w"

  typedef adjacency_list < vecS, vecS, directedS > Graph;
  typedef graph_traits < Graph >::vertex_descriptor Vertex;
  typedef graph_traits < Graph >::vertices_size_type size_type;
  typedef std::pair < size_type, size_type > Pair;
  enum verts
  { a, b, c, d, e, f };
  const char name[] = "abcdef";
  Pair edges[7] = { Pair(a, b), Pair(c, e), Pair(c, f), Pair(a, d),
    Pair(b, e), Pair(e, d), Pair(f, f)
  };
  Graph G(6, edges, edges + 7);
  std::vector < Vertex > v;

  topological_sort(G, std::back_inserter(v));

  std::cout << "A topological ordering: ";
  std::vector < Vertex >::reverse_iterator ii;
  for (ii = v.rbegin(); ii != v.rend(); ++ii)
    std::cout << name[*ii] << " ";
  std::cout << std::endl;

#line 154 "topological-sort.w"

  std::ofstream file("figs/topo-sort-eg.dot");
  write_graphviz(file, G, make_label_writer(name),
                 default_writer(), format_writer());
  return 0;
}
