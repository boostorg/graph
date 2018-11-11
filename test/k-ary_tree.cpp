#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <boost/test/minimal.hpp>


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

int test_main(int, char*[])
{
  BOOST_CONCEPT_ASSERT((boost::concepts::Graph<boost::forward_binary_tree>));
  // BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraph<boost::forward_binary_tree>));


  empty_forward_binary_tree();
  empty_bidirectional_binary_tree();
  push_pop_binary_tree<boost::bidirectional_binary_tree>();
  insert_remove_randomly<boost::bidirectional_binary_tree>();

  return 0;
}
