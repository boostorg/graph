#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <boost/test/minimal.hpp>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>


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
}

template <typename Tree>
void incidence_graph()
{
  Tree tree;
  typedef typename boost::graph_traits<Tree>::vertex_descriptor vertex_descriptor;

  vertex_descriptor u = add_vertex(tree);
  BOOST_CHECK(boost::distance(out_edges(u, tree)) == Tree::k);
  BOOST_CHECK(out_degree(u, tree) == 0);
}

void bidirectional_graph()
{
  boost::bidirectional_binary_tree tree;
  typedef boost::graph_traits<boost::bidirectional_binary_tree>::vertex_descriptor vertex_descriptor;
  vertex_descriptor u = add_vertex(tree);
  BOOST_CHECK(boost::distance(in_edges(u, tree)) == 1);
  BOOST_CHECK(in_degree(u, tree) == 0);
}


int test_main(int, char*[])
{
  BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraph<boost::forward_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::BidirectionalGraph<boost::bidirectional_binary_tree>));

  empty_forward_binary_tree();
  empty_bidirectional_binary_tree();
  push_pop_binary_tree<boost::forward_binary_tree>();
  push_pop_binary_tree<boost::bidirectional_binary_tree>();
  insert_remove_randomly<boost::forward_binary_tree>();
  insert_remove_randomly<boost::bidirectional_binary_tree>();
  incidence_graph<boost::forward_binary_tree>();
  incidence_graph<boost::bidirectional_binary_tree>();
  bidirectional_graph();

  return 0;
}
