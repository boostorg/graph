#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/cstdlib.hpp>

struct edge_prop {
  float weight;
};

int
main()
{
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::bidirectionalS,
    boost::no_property, edge_prop> graph;
  typedef boost::graph_traits<graph>::edge_descriptor edge;

  graph g(2);

  edge_prop p = { 3.14159 };
  edge e; bool b;
  tie(e, b) = add_edge(0, 1, p, g);
  std::cout << "num_edges: " << num_edges(g) << std::endl;
  
  remove_edge(e, g);
  std::cout << "num_edges: " << num_edges(g) << std::endl;

  return boost::exit_success;
}
