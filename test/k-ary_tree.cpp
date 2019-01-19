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

#include <boost/test/minimal.hpp>
#include <boost/array.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

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


void empty_forward_binary_tree()
{
  boost::forward_binary_tree tree;
  BOOST_CHECK(num_vertices(tree) == 0);
}

void empty_bidirectional_binary_tree()
{
  boost::bidirectional_binary_tree tree;
  BOOST_CHECK(num_vertices(tree) == 0);
}

template <typename Tree>
void push_pop_binary_tree()
{
  Tree tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;
  vertex_descriptor u = add_vertex(tree);
  BOOST_ASSERT(num_vertices(tree) == 1);
  remove_vertex(u, tree);
  BOOST_ASSERT(num_vertices(tree) == 0);

  std::vector<vertex_descriptor> added;
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  BOOST_ASSERT(num_vertices(tree) == 3);
  remove_vertex(added.back(), tree);
  added.pop_back();
  remove_vertex(added.back(), tree);
  added.pop_back();
  remove_vertex(added.back(), tree);
  added.pop_back();
  BOOST_ASSERT(num_vertices(tree) == 0);
}


template <typename Tree>
void insert_remove_randomly()
{
  Tree tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  std::vector<vertex_descriptor> added;
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  BOOST_ASSERT(num_vertices(tree) == 3);
  remove_vertex(added[0], tree);
  BOOST_ASSERT(num_vertices(tree) == 2);
  remove_vertex(added[1], tree);
  BOOST_ASSERT(num_vertices(tree) == 1);
  added[0] = add_vertex(tree);
  BOOST_ASSERT(num_vertices(tree) == 2);
  added[1] = add_vertex(tree);
  BOOST_ASSERT(num_vertices(tree) == 3);
  add_edge(added[0], added[1], tree);
  add_edge(4, 5, tree);
}

template <typename Tree>
void incidence_graph()
{
  Tree tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  vertex_descriptor u = add_vertex(tree);
  BOOST_CHECK(boost::distance(out_edges(u, tree)) == 0);
  BOOST_CHECK(out_degree(u, tree) == 0);
  add_edge(u, u + 1, tree);
  BOOST_CHECK(boost::distance(out_edges(u, tree)) == 1);
  BOOST_CHECK(out_degree(u, tree) == 1);
}

void bidirectional_graph()
{
  boost::bidirectional_binary_tree tree;
  create_full_tree(tree, 3);
  BOOST_CHECK(boost::distance(in_edges(0, tree)) == 0);
  BOOST_CHECK(in_degree(0, tree) == 0);
  BOOST_CHECK(boost::distance(in_edges(1, tree)) == 1);
  BOOST_CHECK(in_degree(1, tree) == 1);
  BOOST_CHECK(boost::distance(in_edges(2, tree)) == 1);
  BOOST_CHECK(in_degree(2, tree) == 1);
  BOOST_CHECK(root(0, tree) == 0);
  BOOST_CHECK(root(1, tree) == 0);
  BOOST_CHECK(root(2, tree) == 0);
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


template <bool Predecessor>
void depth_first_search()
{
  typedef boost::k_ary_tree<2, Predecessor> Tree;
  boost::k_ary_tree<2, Predecessor> tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 3);

  std::vector<boost::default_color_type> color;
  typedef std::pair<boost::order::visit, vertex_descriptor> visiting;
  boost::array< visiting, 9> const expected_seq =
  {
    std::make_pair(boost::order::pre, 0),
    std::make_pair(boost::order::pre, 1),
    std::make_pair(boost::order::in, 1),
    std::make_pair(boost::order::post, 1),
    std::make_pair(boost::order::in, 0),
    std::make_pair(boost::order::pre, 2),
    std::make_pair(boost::order::in, 2),
    std::make_pair(boost::order::post, 2),
    std::make_pair(boost::order::post, 0)
  };
  tree_visitor<boost::order::visit, vertex_descriptor> visitor;
  depth_first_visit(tree, vertex_descriptor(0), visitor, color);
  BOOST_CHECK(boost::equal(visitor.visited, expected_seq));
}


void mutable_bidirectional()
{
  typedef boost::k_ary_tree<2, true> Tree;
  Tree tree;
  typedef boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 3);

  BOOST_CHECK(predecessor(vertex_descriptor(1), tree) == 0);
  BOOST_CHECK(predecessor(vertex_descriptor(2), tree) == 0);

  BOOST_CHECK(has_left_successor(vertex_descriptor(0), tree));
  BOOST_CHECK(has_right_successor(vertex_descriptor(0), tree));

  clear_vertex(vertex_descriptor(0), tree);

  BOOST_CHECK(num_vertices(tree) == 3);

  vertex_descriptor null = boost::graph_traits<Tree>::null_vertex();

  BOOST_CHECK(predecessor(vertex_descriptor(1), tree) == null);
  BOOST_CHECK(predecessor(vertex_descriptor(2), tree) == null);

  BOOST_CHECK(!has_left_successor(vertex_descriptor(0), tree));
  BOOST_CHECK(!has_right_successor(vertex_descriptor(0), tree));

  // Remove after clearing.
  remove_vertex(vertex_descriptor(0), tree);

  BOOST_CHECK(num_vertices(tree) == 2);

}


template <bool Predecessor>
void binary_tree()
{
  typedef boost::k_ary_tree<2, Predecessor> Tree;
  boost::k_ary_tree<2, Predecessor> tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  std::vector<vertex_descriptor> added;

  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  added.push_back(add_vertex(tree));
  add_edge(added[0], added[1], tree);
  add_edge(added[0], added[2], tree);
  BOOST_CHECK(has_left_successor(added[0], tree));
  BOOST_CHECK(has_right_successor(added[0], tree));
  BOOST_CHECK(!has_left_successor(added[1], tree));
  BOOST_CHECK(!has_right_successor(added[1], tree));
  BOOST_CHECK(!has_left_successor(added[2], tree));
  BOOST_CHECK(!has_right_successor(added[2], tree));

  BOOST_CHECK(isomorphism(tree, tree));

  remove_edge(added[0], added[1], tree);
  remove_edge(added[0], added[2], tree);
  BOOST_CHECK(!has_left_successor(added[0], tree));
  BOOST_CHECK(!has_right_successor(added[0], tree));
}


template <bool Predecessor>
void VertexListGraph_test()
{
  typedef boost::k_ary_tree<2, Predecessor> Tree;
  Tree tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  create_full_tree(tree, 7);

  boost::array<vertex_descriptor, 7> actual,
                                      expected = {3, 1, 4, 0, 5, 2, 6};
  copy(vertices(tree), boost::begin(actual));
  BOOST_CHECK(actual == expected);
}

int test_main(int, char*[])
{
  BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraph<boost::forward_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::BidirectionalGraph<boost::bidirectional_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::MutableGraph<boost::bidirectional_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::MutableGraph<boost::forward_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::VertexListGraph<boost::forward_binary_tree>));

  empty_forward_binary_tree();
  empty_bidirectional_binary_tree();
  push_pop_binary_tree<boost::forward_binary_tree>();
  push_pop_binary_tree<boost::bidirectional_binary_tree>();
  insert_remove_randomly<boost::forward_binary_tree>();
  insert_remove_randomly<boost::bidirectional_binary_tree>();
  incidence_graph<boost::forward_binary_tree>();
  incidence_graph<boost::bidirectional_binary_tree>();
  bidirectional_graph();

  binary_tree<0>();
  binary_tree<1>();

  depth_first_search<0>();
  depth_first_search<1>();

  mutable_bidirectional();

  VertexListGraph_test<0>();
  // VertexListGraph_test<1>();

  return 0;
}
