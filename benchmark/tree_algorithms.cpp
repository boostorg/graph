// #include <boost/graph/graphviz.hpp>
#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/visitors.hpp>
#include <boost/graph/isomorphism.hpp>
// #include <boost/graph/vf2_sub_graph_iso.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_matrix.hpp>
#include <boost/graph/property_maps/container_property_map.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>

#include <benchmark/benchmark.h>

using namespace boost;
using namespace boost::graph;

#ifdef NDEBUG

  template <typename Graph>
  void create_tree(Graph &tree, vertex_descriptor_t<Graph> weight)
  {
    BOOST_ASSERT(weight >= 0);
    if (weight == 0) return;
    if (weight == 1) {
      add_vertex(tree);
      return;
    }

    typedef vertex_descriptor_t<Graph> vertex_descriptor;
    vertex_descriptor parent = 0;
    for (vertex_descriptor child = 1; child != weight; child++) {
      add_edge(parent, child, tree);
      if (!(child & 1))
        parent++;
    }
  }


  template <typename BinaryTree>
  void create_binary_tree(BinaryTree &tree,
                          vertex_descriptor_t<BinaryTree> weight)
  {
    BOOST_ASSERT(weight >= 0);

    tree = BinaryTree(weight);
    typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;
    vertex_descriptor parent = 0;
    for (vertex_descriptor child = 1; child < weight; child++)
    {
      if (child % 2 == 1)
        add_left_edge(parent, child, tree);
      else
        add_right_edge(parent++, child, tree);
    }
  }



void create_tree(compressed_sparse_row_graph<> &tree,
                 vertex_descriptor_t<compressed_sparse_row_graph<>> weight)
{
  BOOST_ASSERT(weight >= 0);

  if (weight == 0)
    return;

  adjacency_list<> tmp;

  auto parent = add_vertex(tmp);
  if (weight == 1)
    return;

  for (auto child = parent + 1; child != weight; child++)
  {
    add_edge(parent, child, tmp);
    if (child % 2 == 0)
      parent++;
  }

  tree = compressed_sparse_row_graph<>(tmp);
}


template <typename BinaryTree>
vertex_descriptor_t<BinaryTree>
create_binary_subtree(BinaryTree &tree, vertex_descriptor_t<BinaryTree> parent,
                      vertex_descriptor_t<BinaryTree> child, int depth)
{
  if (depth == 0)
    return child;

  add_left_edge(parent, child, tree);
  child = create_binary_subtree(tree, child, child + 1, depth - 1);
  add_right_edge(parent, child, tree);
  return create_binary_subtree(tree, child, child + 1, depth - 1);
}


template <typename BinaryTree>
void create_binary_tree_recursive(BinaryTree &tree, int depth)
{
  BOOST_ASSERT(depth >= 0);

  if (depth == 0)
    return;

  vertex_descriptor_t<BinaryTree> const weight = (1 << depth) - 1;
  tree = BinaryTree(weight);
  auto result = create_binary_subtree(tree, 0, 1, depth - 1);
  BOOST_ASSERT(result == weight);
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


template <typename Graph>
static void BM_create_tree(benchmark::State &s)
{
  while (s.KeepRunning())
  {
    Graph g;
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

template <typename Integer>
std::enable_if_t<sizeof(Integer) == 4, char>
nlz(Integer x)
{
  int n;
  if (x == 0) return(32);
  n = 1;
  if ((x >> 16) == 0) {n = n +16; x = x <<16;}
  if ((x >> 24) == 0) {n = n + 8; x = x << 8;}
  if ((x >> 28) == 0) {n = n + 4; x = x << 4;}
  if ((x >> 30) == 0) {n = n + 2; x = x << 2;}
  n = n - (x >> 31);
  return n;
}

template <typename Integer>
std::enable_if_t<sizeof(Integer) == 8
                && !std::numeric_limits<Integer>::is_signed, char>
nlz(Integer x)
{
  if (x == 0) return(32);
  int n = 0;
  if (x <= 0x00000000FFFFFFFF) {n = n +32; x = x <<32;}
  if (x <= 0x0000FFFFFFFFFFFF) {n = n + 16; x = x << 16;}
  if (x <= 0x00FFFFFFFFFFFFFF) {n = n + 8; x = x << 8;}
  if (x <= 0x0FFFFFFFFFFFFFFF) {n = n + 4; x = x << 4;}
  if (x <= 0x3FFFFFFFFFFFFFFF) {n = n + 2; x = x << 2;}
  if (x <= 0x7FFFFFFFFFFFFFFF) {n = n + 1;}
  return n;
}


template <typename Integer>
std::enable_if_t<sizeof(Integer) == 4, char>
ilog2(Integer x)
{
  return 31 - nlz(x);
}

template <typename Integer>
std::enable_if_t<sizeof(Integer) == 8
                && !std::numeric_limits<Integer>::is_signed, char>
ilog2(Integer x)
{
  return 63 - nlz(x);
}


template <typename BinaryTree>
static void BM_create_binary_tree_recursive(benchmark::State &s)
{
  while (s.KeepRunning())
  {
    BinaryTree g;
    create_binary_tree_recursive(g, ilog2(static_cast<std::uint64_t>(s.range(0))));
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

template <typename Graph>
static void BM_depth_first_search_graph(benchmark::State &s)
{
  Graph g;

  create_tree(g, s.range(0) - 1);

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

int const dfs_min = 8, dfs_max = 8<<17;

using adjacency_list_vec = adjacency_list<vecS, vecS, directedS, no_property,
no_property, no_property, vecS>;

BENCHMARK_TEMPLATE(BM_create_tree,
                   adjacency_list<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_create_binary_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);

BENCHMARK_TEMPLATE(BM_depth_first_search_binary_tree,
                   forward_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_depth_first_search_binary_tree,
                   bidirectional_binary_tree<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_depth_first_search_graph, adjacency_list<>)->Range(dfs_min, dfs_max);
BENCHMARK_TEMPLATE(BM_depth_first_search_graph, compressed_sparse_row_graph<>)->Range(dfs_min, dfs_max);

BENCHMARK_TEMPLATE(BM_graph_isomorphism, forward_binary_tree<>)->Range(2, 2<<17);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, bidirectional_binary_tree<>)->Range(2, 2<<17);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, adjacency_list<>)->Range(2, 2<<13);
BENCHMARK_TEMPLATE(BM_graph_isomorphism, compressed_sparse_row_graph<>)->Range(2, 2<<13);

#endif

BENCHMARK_MAIN();
