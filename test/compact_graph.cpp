#define BOOST_INCLUDE_MAIN
#include <boost/test/test_tools.hpp>

#include <siek_info/graph/compact_graph.hpp>
#include <boost/graph/graph_test.hpp>
#include <boost/graph/graph_utility.hpp>

int test_main(int, char*[])
{
  typedef siek_info::compact_graph<boost::bidirectionalS> graph_t;
  boost::graph_test<graph_t> gt;

  const std::size_t N = 3;
  std::vector<std::size_t> vertex_set(N);
  for (std::size_t i = 0; i < N; ++i)
    vertex_set[i] = i;

  std::vector<std::pair<std::size_t,std::size_t> > edge_set;
  edge_set.push_back(std::make_pair(0, 1));
  edge_set.push_back(std::make_pair(1, 2));
  edge_set.push_back(std::make_pair(1, 0));
  edge_set.push_back(std::make_pair(2, 0));
  edge_set.push_back(std::make_pair(2, 0));

  graph_t g(edge_set.begin(), edge_set.end(), N);

  print_graph(g, boost::identity_property_map());
  
  gt.test_incidence_graph(vertex_set, edge_set, g);
  gt.test_bidirectional_graph(vertex_set, edge_set, g);
  gt.test_adjacency_graph(vertex_set, edge_set, g);
  gt.test_vertex_list_graph(vertex_set, g);
  gt.test_edge_list_graph(vertex_set, edge_set, g);
  
  // test vertex and edge properties...
  
  return EXIT_SUCCESS;
}
