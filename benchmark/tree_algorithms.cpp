#include <boost/graph/graphviz.hpp>
#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/isomorphism.hpp>
// #include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/property_maps/container_property_map.hpp>

#include <benchmark/benchmark.h>

using namespace boost;

template <typename Graph>
void create_tree(Graph &tree, typename graph_traits<Graph>::vertex_descriptor weight)
{
  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

  vertex_descriptor parent = 0;
  for (vertex_descriptor child = 1; child != weight; child++)
  {
    add_edge(parent, child, tree);
    if (!(child & 1))
      parent++;
  }
}

template <typename Order, typename Vertex>
struct tree_visitor
{
  void operator()(Order, Vertex)
  {
    // visited.push_back(std::make_pair(visit, u));
  }

  std::vector< std::pair<Order, Vertex> > visited;
};


static void BM_depth_first_visit_bidirectional_binary_tree(benchmark::State &s)
{
  typedef graph_traits<bidirectional_binary_tree>::vertex_descriptor vertex_descriptor;

  bidirectional_binary_tree g;
  std::vector<boost::default_color_type> color;

  create_tree(g, s.range(0));

  while (s.KeepRunning())
  {
    tree_visitor<boost::order::visit, vertex_descriptor> visitor;
    depth_first_visit(g, vertex_descriptor(), visitor, color);
  }
}

static void BM_depth_first_visit_adjacency_list(benchmark::State &s)
{
  adjacency_list<> g;

  create_tree(g, s.range(0));

  while (s.KeepRunning())
  {
    depth_first_search(g, visitor(dfs_visitor<>()));
  }
}

template <typename Graph>
static void BM_graph_isomorphism(benchmark::State &s)
{
  Graph g;
  create_tree(g, s.range(0));

  while (s.KeepRunning())
  {
    benchmark::DoNotOptimize(static_cast<bool(*)(Graph const &, Graph const &)>(boost::isomorphism)(g, g));
  }
}

// template <typename Graph>
// static void BM_graph_vf2_isomorphism(benchmark::State &s)
// {
//   Graph g(s.range(0));
//   create_tree(g, s.range(0));
//   vf2_print_callback<Graph, Graph> callback(g, g);
//
//   while (s.KeepRunning())
//   {
//     benchmark::DoNotOptimize(vf2_subgraph_iso(g, g, callback));
//   }
// }

BENCHMARK_TEMPLATE(BM_graph_isomorphism, bidirectional_binary_tree)->Range(8, 8<<14);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, adjacency_list<>)->Range(8, 8<<10);
// BENCHMARK_TEMPLATE(BM_graph_vf2_isomorphism, adjacency_matrix<>)->Range(8, 8<<22);

BENCHMARK(BM_depth_first_visit_bidirectional_binary_tree)->Range(8, 8<<16);
BENCHMARK(BM_depth_first_visit_adjacency_list)->Range(8, 8<<16);


BENCHMARK_MAIN();
