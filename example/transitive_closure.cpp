// Copyright (c) Jeremy Siek 2001
//
// Permission to use, copy, modify, distribute and sell this software
// and its documentation for any purpose is hereby granted without fee,
// provided that the above copyright notice appears in all copies and
// that both that copyright notice and this permission notice appear
// in supporting documentation.  Silicon Graphics makes no
// representations about the suitability of this software for any
// purpose.  It is provided "as is" without express or implied warranty.

#include <boost/graph/transitive_closure.hpp>
#include <boost/graph/graphviz.hpp>

int main(int, char*[])
{
  using namespace boost;
  char name[] = "abcd";
  GraphvizDigraph G;
  read_graphviz("tc.dot", G);

  std::cout << "Graph G:" << std::endl;
  print_graph(G, name);

  transitive_closure(G);

  std::cout << std::endl << "Graph G*:" << std::endl;
  print_graph(G, name);
  std::cout << std::endl;

  write_graphviz("tc-out.dot", G);

  return 0;
}
