// (C) Copyright Jeremy Siek 2001. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#include <boost/graph/isomorphism.hpp>
#include <boost/graph/adjacency_list.hpp>

int
main()
{
  using namespace boost;
  
  const int n = 12;
  adjacency_list<vecS, vecS, undirectedS> g1(n), g2(n);
  add_edge(0, 1, g1);  add_edge(1, 2, g1);  add_edge(0, 2, g1);
  add_edge(3, 4, g1);  add_edge(4, 5, g1);
  add_edge(5, 6, g1);  add_edge(6, 3, g1);
  add_edge(7, 8, g1);  add_edge(8, 9, g1);  add_edge(9, 10, g1);
  add_edge(10, 11, g1);  add_edge(11, 7, g1);

  add_edge(9, 10, g2);  add_edge(10, 11, g2);  add_edge(11, 9, g2);
  add_edge(0, 1, g2);  add_edge(1, 3, g2); 
  add_edge(3, 2, g2);  add_edge(2, 0, g2);
  add_edge(4, 5, g2); add_edge(5, 7, g2); add_edge(7, 8, g2);
  add_edge(8, 6, g2); add_edge(6, 4, g2);

  std::vector<int> f(n);

  bool ret = isomorphism(g1, g2, f.begin());
  std::cout << "isomorphic? " << ret << std::endl;

  std::cout << "f: ";
  std::copy(f.begin(), f.end(), std::ostream_iterator<int>(std::cout, " "));
  std::cout << std::endl;
  
  return 0;
}
