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
using namespace boost::graph;

#ifdef NDEBUG

template <typename Graph>
void create_tree(Graph &tree, typename graph_traits<Graph>::vertex_descriptor weight)
{
  BOOST_ASSERT(weight >= 0);

  if (weight == 0)
    return;

  if (weight == 1)
  {
    add_vertex(tree);
    return;
  }

  typedef typename graph_traits<Graph>::vertex_descriptor vertex_descriptor;

  vertex_descriptor parent = 0;
  for (vertex_descriptor child = 1; child != weight; child++)
  {
    add_edge(parent, child, tree);
    if (!(child & 1))
      parent++;
  }
}

template <typename BinaryTree>
void create_binary_tree(BinaryTree &tree,
                        typename graph_traits<BinaryTree>::vertex_descriptor weight)
{
  BOOST_ASSERT(weight >= 0);

  tree = BinaryTree(weight);
  typedef typename graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;
  vertex_descriptor parent = 0;
  for (vertex_descriptor child = 1; child < weight; child++)
  {
    if (child % 2 == 1)
      add_left_edge(parent, child, tree);
    else
      add_right_edge(parent++, child, tree);
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


template <typename BinaryTree>
static void BM_create_tree(benchmark::State &s)
{
  while (s.KeepRunning())
  {
    BinaryTree g;
    create_tree(g, s.range(0) - 1);
    benchmark::DoNotOptimize(g);
  }
}


template <typename BinaryTree>
static void BM_create_binary_tree(benchmark::State &s)
{
  while (s.KeepRunning())
  {
    BinaryTree g;
    create_binary_tree(g, s.range(0) - 1);
    benchmark::DoNotOptimize(g);
  }
}


template <typename BinaryTree>
static void BM_depth_first_search_binary_tree(benchmark::State &s)
{
  typedef typename graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  BinaryTree g;

  create_tree(g, s.range(0) - 1);

  while (s.KeepRunning())
  {
    tree_visitor<boost::visit, vertex_descriptor> visitor;
    depth_first_search(g, vertex_descriptor(), visitor);
  }
}

static void BM_depth_first_search_adjacency_list(benchmark::State &s)
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
    benchmark::DoNotOptimize(boost::isomorphism(g, g));
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

int const dfs_min = 8, dfs_max = 8<<17;

using adjacency_list_vec = adjacency_list<vecS, vecS, directedS, no_property,
no_property, no_property, vecS>;

BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   forward_binary_tree<unsigned>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   bidirectional_binary_tree<unsigned>)->Range(dfs_min, dfs_max);

BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_tree,
                   adjacency_list<>)->Range(dfs_min, dfs_max);


BENCHMARK_TEMPLATE(BM_create_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);
/*
// Same as default.
BENCHMARK_TEMPLATE(BM_create_tree,
                   adjacency_list_vec)->Range(dfs_min, dfs_max);
*/

BENCHMARK_TEMPLATE(BM_depth_first_search_binary_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_depth_first_search_binary_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK(BM_depth_first_search_adjacency_list)->Range(dfs_min, dfs_max);

BENCHMARK_TEMPLATE(BM_graph_isomorphism, forward_binary_tree<>)->Range(2, 2<<17);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, bidirectional_binary_tree<>)->Range(2, 2<<17);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, adjacency_list<>)->Range(2, 2<<13);
/*
*/
// BENCHMARK_TEMPLATE(BM_graph_vf2_isomorphism, adjacency_matrix<>)->Range(8, 8<<22);



BENCHMARK_MAIN();

#endif
