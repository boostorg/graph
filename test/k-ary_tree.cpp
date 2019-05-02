//=======================================================================
// Copyright 2018 Jeremy William Murphy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>

#define BOOST_TEST_MODULE k-ary tree
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

#include <array>
#include <tuple>
#include <utility>

template <typename Graph>
void create_full_tree(Graph &tree,
                      typename boost::graph_traits<Graph>::vertex_descriptor weight)
{
  typedef typename boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;

  vertex_descriptor parent = 0;
  for (vertex_descriptor child = 1; child != weight; child++)
  {
    add_edge(parent, child, tree);
    if (!(child & 1))
      parent++;
  }
}

using tree_types = std::tuple<boost::forward_binary_tree,
                              boost::bidirectional_binary_tree>;

BOOST_AUTO_TEST_CASE_TEMPLATE(empty_tree, BinaryTree, tree_types)
{
  BinaryTree tree;
  BOOST_TEST(num_vertices(tree) == 0);
}


BOOST_AUTO_TEST_CASE_TEMPLATE(push_pop, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;
  vertex_descriptor u = add_vertex(tree);
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
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  std::vector<vertex_descriptor> added;
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  BOOST_TEST(num_vertices(tree) == 3);
  remove_vertex(added[0], tree);
  BOOST_TEST(num_vertices(tree) == 2);
  remove_vertex(added[1], tree);
  BOOST_TEST(num_vertices(tree) == 1);
  added[0] = add_vertex(tree);
  BOOST_TEST(num_vertices(tree) == 2);
  added[1] = add_vertex(tree);
  BOOST_TEST(num_vertices(tree) == 3);
  add_edge(added[0], added[1], tree);
  add_edge(4, 5, tree);
}

BOOST_AUTO_TEST_CASE_TEMPLATE(incidence_graph, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  vertex_descriptor u = add_vertex(tree);
  BOOST_TEST(boost::distance(out_edges(u, tree)) == 0);
  BOOST_TEST(out_degree(u, tree) == 0);
  add_edge(u, u + 1, tree);
  BOOST_TEST(boost::distance(out_edges(u, tree)) == 1);
  BOOST_TEST(out_degree(u, tree) == 1);
}

BOOST_AUTO_TEST_CASE(bidirectional)
{
  boost::bidirectional_binary_tree tree;
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
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 3);

  std::vector<boost::default_color_type> color;
  typedef std::pair<boost::order::visit, vertex_descriptor> visiting;
  std::array< visiting, 9> const expected_seq =
  {{
    std::make_pair(boost::order::pre, 0),
    std::make_pair(boost::order::pre, 1),
    std::make_pair(boost::order::in, 1),
    std::make_pair(boost::order::post, 1),
    std::make_pair(boost::order::in, 0),
    std::make_pair(boost::order::pre, 2),
    std::make_pair(boost::order::in, 2),
    std::make_pair(boost::order::post, 2),
    std::make_pair(boost::order::post, 0)
  }};
  tree_visitor<boost::order::visit, vertex_descriptor> visitor;
  depth_first_visit(tree, 0, visitor, color);
  BOOST_TEST(boost::equal(visitor.visited, expected_seq));
}


BOOST_AUTO_TEST_CASE(mutable_bidirectional)
{
  typedef boost::k_ary_tree<2, true> BinaryTree;
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

  // Remove after clearing.
  remove_vertex(vertex_descriptor(0), tree);

  BOOST_TEST(num_vertices(tree) == 2);

}


BOOST_AUTO_TEST_CASE_TEMPLATE(Mutable, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

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

  BOOST_TEST(isomorphism(tree, tree));

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
}


BOOST_AUTO_TEST_CASE_TEMPLATE(VertexListGraph, BinaryTree, tree_types)
{
  BinaryTree tree;
  typedef typename boost::graph_traits<BinaryTree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 7);

  std::array<vertex_descriptor, 7> actual,
                                   expected = {{3, 1, 4, 0, 5, 2, 6}};
  copy(vertices(tree), begin(actual));
  BOOST_TEST(actual == expected);
}

using namespace boost::concepts;
using boost::forward_binary_tree;
using boost::bidirectional_binary_tree;

BOOST_CONCEPT_ASSERT((IncidenceGraphConcept<forward_binary_tree>));
BOOST_CONCEPT_ASSERT((BidirectionalGraphConcept<bidirectional_binary_tree>));
BOOST_CONCEPT_ASSERT((MutableGraphConcept<bidirectional_binary_tree>));
BOOST_CONCEPT_ASSERT((MutableGraphConcept<forward_binary_tree>));
BOOST_CONCEPT_ASSERT((VertexListGraphConcept<forward_binary_tree>));
BOOST_CONCEPT_ASSERT((VertexListGraphConcept<bidirectional_binary_tree>));
// BOOST_CONCEPT_ASSERT((EdgeListGraph<forward_binary_tree>));
// BOOST_CONCEPT_ASSERT((EdgeListGraph<bidirectional_binary_tree>));
