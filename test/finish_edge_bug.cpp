// Author: Alex Lauser

// Output:
// The example graph:
// 0 --> 1 2
// 1 --> 2
// 2 --> 0

#include <boost/config.hpp>
#include <iostream>
#include <vector>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>
#include <boost/core/lightweight_test.hpp>


template<typename graph_t>
struct TalkativeVisitor
  : boost::dfs_visitor<>
{
  typedef typename boost::graph_traits<graph_t>::vertex_descriptor vertex_descriptor;
  typedef typename boost::graph_traits<graph_t>::edge_descriptor edge_descriptor;
};

template <typename t>
std::ostream &operator<<(std::ostream &os, const std::pair<t,t> &x) {
  return os << "(" << x.first << ", " << x.second << ")";
}


int main(int, char*[])
{
  using namespace boost;

  typedef  adjacency_list<vecS, vecS, directedS> Graph;
  Graph G;

  typedef graph_traits<adjacency_list<vecS, vecS, directedS> >::vertex_descriptor Vertex;
  Vertex a = add_vertex(G);
  Vertex b = add_vertex(G);
  Vertex c = add_vertex(G);

  add_edge(a, b, G);
  add_edge(b, c, G);
  add_edge(c, a, G);
  add_edge(a, c, G);

  print_graph(G);

  std::vector<default_color_type> color(num_vertices(G));
  depth_first_search(G, visitor(TalkativeVisitor<Graph>()));

  return boost::report_errors();
}
