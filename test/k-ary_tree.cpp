//=======================================================================
// Copyright 2018 Jeremy William Murphy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#if __cplusplus > 201103L


#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>

#define BOOST_TEST_MODULE k-ary tree
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

#include <boost/concept/assert.hpp>

#include <boost/assert.hpp>

#include <array>
#include <tuple>
#include <utility>

using boost::vertex_descriptor_t;

template <typename Graph>
void create_full_tree(Graph &tree, vertex_descriptor_t<Graph> weight)
{
  BOOST_ASSERT(weight >= 0);

  if (weight == 0)
    return;

  if (weight == 1)
  {
    add_vertex(tree);
    return;
  }

  typedef vertex_descriptor_t<Graph> vertex_descriptor;

  vertex_descriptor parent = 0;
  for (vertex_descriptor child = 1; child != weight; child++)
  {
    add_edge(parent, child, tree);
    if (!(child & 1))
      parent++;
  }
}

using tree_types = std::tuple<boost::forward_binary_tree<>,
                              boost::bidirectional_binary_tree<>>;

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_tree, BinaryTree, tree_types)
{
  BinaryTree tree;
  BOOST_TEST(num_vertices(tree) == 0);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(push_pop, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;
  auto u = add_vertex(tree);
  BOOST_TEST(num_vertices(tree) == 1);
  remove_vertex(u, tree);
  BOOST_TEST(num_vertices(tree) == 0);

  std::vector<vertex_descriptor> added;
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  BOOST_TEST(num_vertices(tree) == 3);
  remove_vertex(added.back(), tree);
  added.pop_back();
  remove_vertex(added.back(), tree);
  added.pop_back();
  remove_vertex(added.back(), tree);
  added.pop_back();
  BOOST_TEST(num_vertices(tree) == 0);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(insert_remove_randomly, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;

  std::vector<vertex_descriptor> added;
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  BOOST_TEST(num_vertices(tree) == 3);
  remove_vertex(added[2], tree);
  BOOST_TEST(num_vertices(tree) == 2);
  remove_vertex(added[1], tree);
  BOOST_TEST(num_vertices(tree) == 1);
  added[2] = add_vertex(tree);
  BOOST_TEST(num_vertices(tree) == 2);
  added[1] = add_vertex(tree);
  BOOST_TEST(num_vertices(tree) == 3);
  add_edge(added[0], added[1], tree);
  add_edge(4, 5, tree);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(incidence_graph, BinaryTree, tree_types)
{
  BinaryTree tree;

  auto u = add_vertex(tree);
  BOOST_TEST(boost::distance(out_edges(u, tree)) == 0);
  BOOST_TEST(out_degree(u, tree) == 0);
  add_edge(u, u + 1, tree);
  BOOST_TEST(boost::distance(out_edges(u, tree)) == 1);
  BOOST_TEST(out_degree(u, tree) == 1);
}

BOOST_AUTO_TEST_CASE(bidirectional)
{
  boost::bidirectional_binary_tree<> tree;
  create_full_tree(tree, 5);
  BOOST_TEST(boost::distance(in_edges(0, tree)) == 0);
  BOOST_TEST(in_degree(0, tree) == 0);
  BOOST_TEST(boost::distance(in_edges(1, tree)) == 1);
  BOOST_TEST(in_degree(1, tree) == 1);
  BOOST_TEST(boost::distance(in_edges(2, tree)) == 1);
  BOOST_TEST(in_degree(2, tree) == 1);
  BOOST_TEST(root(0, tree) == 0);
  BOOST_TEST(root(1, tree) == 0);
  BOOST_TEST(root(2, tree) == 0);
  BOOST_TEST(root(3, tree) == 0);
  BOOST_TEST(root(4, tree) == 0);
  // detach the (1(3, 4)) subtree.
  remove_edge(0, 1, tree);
  BOOST_TEST(!has_left_successor(0, tree));
  BOOST_TEST(root(0, tree) == 0);
  BOOST_TEST(root(1, tree) == 1);
  BOOST_TEST(root(2, tree) == 0);
  BOOST_TEST(root(3, tree) == 1);
  BOOST_TEST(root(4, tree) == 1);
}

template <typename Order, typename Vertex>
struct tree_visitor
{
  void operator()(Order visit, Vertex u)
  {
    visited.push_back(std::make_pair(visit, u));
  }

  std::vector< std::pair<Order, Vertex> > visited;
};

BOOST_AUTO_TEST_CASE_TEMPLATE(test_depth_first_search, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;

  create_full_tree(tree, 3);

  using boost::visit;
  typedef std::pair<visit, vertex_descriptor> visiting;
  std::array< visiting, 9> const expected_seq
  {{
    std::make_pair(visit::pre, 0),
    std::make_pair(visit::pre, 1),
    std::make_pair(visit::in, 1),
    std::make_pair(visit::post, 1),
    std::make_pair(visit::in, 0),
    std::make_pair(visit::pre, 2),
    std::make_pair(visit::in, 2),
    std::make_pair(visit::post, 2),
    std::make_pair(visit::post, 0)
  }};
  tree_visitor<visit, vertex_descriptor> visitor;
  depth_first_search(tree, 0, visitor);
  BOOST_TEST(boost::equal(visitor.visited, expected_seq));
}


BOOST_AUTO_TEST_CASE(mutable_bidirectional)
{
  typedef boost::binary_tree<true> BinaryTree;
  BinaryTree tree;
  typedef boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 3);

  BOOST_TEST(predecessor(vertex_descriptor(1), tree) == 0);
  BOOST_TEST(predecessor(vertex_descriptor(2), tree) == 0);

  BOOST_TEST(has_left_successor(vertex_descriptor(0), tree));
  BOOST_TEST(has_right_successor(vertex_descriptor(0), tree));

  clear_vertex(vertex_descriptor(0), tree);

  BOOST_TEST(num_vertices(tree) == 3);

  vertex_descriptor null = boost::graph_traits<BinaryTree>::null_vertex();

  BOOST_TEST(predecessor(vertex_descriptor(1), tree) == null);
  BOOST_TEST(predecessor(vertex_descriptor(2), tree) == null);

  BOOST_TEST(!has_left_successor(vertex_descriptor(0), tree));
  BOOST_TEST(!has_right_successor(vertex_descriptor(0), tree));

  auto u = add_vertex(tree);
  add_left_edge(0, u, tree);

  BOOST_TEST(has_left_successor(vertex_descriptor(0), tree));
  BOOST_TEST(left_successor(vertex_descriptor(0), tree) == u);

  add_right_edge(0, u, tree);

  BOOST_TEST(has_right_successor(vertex_descriptor(0), tree));
  BOOST_TEST(right_successor(vertex_descriptor(0), tree) == u);

  // Remove after clearing.
  /*
  remove_vertex(vertex_descriptor(0), tree);

  BOOST_TEST(num_vertices(tree) == 2);
  */
}


BOOST_AUTO_TEST_CASE_TEMPLATE(Mutable, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;

  std::vector<vertex_descriptor> added;

  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  add_edge(added[0], added[1], tree);
  add_edge(added[0], added[2], tree);
  BOOST_TEST(has_left_successor(added[0], tree));
  BOOST_TEST(has_right_successor(added[0], tree));
  BOOST_TEST(!has_left_successor(added[1], tree));
  BOOST_TEST(!has_right_successor(added[1], tree));
  BOOST_TEST(!has_left_successor(added[2], tree));
  BOOST_TEST(!has_right_successor(added[2], tree));

  remove_edge(added[0], added[1], tree);
  remove_edge(added[0], added[2], tree);
  BOOST_TEST(!has_left_successor(added[0], tree));
  BOOST_TEST(!has_right_successor(added[0], tree));
}

BOOST_AUTO_TEST_CASE_TEMPLATE(test_isomorphism, BinaryTree, tree_types)
{
  BinaryTree tree0, tree1;
  BOOST_TEST(isomorphism(tree0, tree1));
  create_full_tree(tree0, 15);
  create_full_tree(tree1, 15);
  BOOST_TEST(isomorphism(tree0, tree1));
  tree0.clear();
  tree1.clear();
  // create in different sequence
  add_edge(0, 1, tree0);
  add_edge(0, 2, tree0);
  add_edge(0, 2, tree1);
  add_edge(0, 1, tree1);
  BOOST_TEST(isomorphism(tree0, tree1));

}

BOOST_AUTO_TEST_CASE_TEMPLATE(VertexListGraph, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef vertex_descriptor_t<BinaryTree> vertex_descriptor;

  create_full_tree(tree, 7);

  std::array<vertex_descriptor, 7> actual,
                                   expected = {{3, 1, 4, 0, 5, 2, 6}};
  boost::copy(vertices(tree), begin(actual));
  BOOST_TEST(actual == expected);
}

using namespace boost::concepts;
using boost::forward_binary_tree;
using boost::bidirectional_binary_tree;

BOOST_CONCEPT_ASSERT((IncidenceGraphConcept<forward_binary_tree<>>));
BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept<bidirectional_binary_tree<>>));
BOOST_CONCEPT_ASSERT((MutableGraphConcept<bidirectional_binary_tree<>>));
BOOST_CONCEPT_ASSERT((MutableGraphConcept<forward_binary_tree<>>));
BOOST_CONCEPT_ASSERT((VertexListGraphConcept<forward_binary_tree<>>));
BOOST_CONCEPT_ASSERT((VertexListGraphConcept<bidirectional_binary_tree<>>));
BOOST_CONCEPT_ASSERT((ForwardBinaryTreeConcept<forward_binary_tree<>>));
BOOST_CONCEPT_ASSERT((MutableForwardBinaryTreeConcept<forward_binary_tree<>>));
BOOST_CONCEPT_ASSERT((BidirectionalBinaryTreeConcept<bidirectional_binary_tree<>>));
// BOOST_CONCEPT_ASSERT((EdgeListGraph<forward_binary_tree<>>));
// BOOST_CONCEPT_ASSERT((EdgeListGraph<bidirectional_binary_tree<>>));

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

BOOST_AUTO_TEST_CASE_TEMPLATE(test_create_binary_tree_recursive, BinaryTree,
                              tree_types)
{
  BinaryTree tree;
  create_binary_tree_recursive(tree, 0);
  BOOST_ASSERT(num_vertices(tree) == 0);
  create_binary_tree_recursive(tree, 1);
  BOOST_ASSERT(num_vertices(tree) == 1);
  create_binary_tree_recursive(tree, 2);
  BOOST_ASSERT(num_vertices(tree) == 3);
  create_binary_tree_recursive(tree, 3);
  BOOST_ASSERT(num_vertices(tree) == 7);
}


#else

int main(int argc, char **argv)
{

}

#endif // #if __cplusplus > 201103L
