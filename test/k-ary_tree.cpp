#include <boost/graph/k-ary_tree.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <boost/test/minimal.hpp>
#include <boost/array.hpp>
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
  added[0] = add_vertex(tree);
  BOOST_ASSERT(num_vertices(tree) == 2);
  added[1] = add_vertex(tree);
  BOOST_ASSERT(num_vertices(tree) == 3);
  add_edge(added[0], added[1], tree);
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
  BOOST_CHECK(boost::distance(in_edges(u, tree)) == 0);
  BOOST_CHECK(in_degree(u, tree) == 0);
  add_edge(u, u, tree);
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

  std::vector<boost::default_color_type> color;
  boost::array< std::pair<boost::visit, vertex_descriptor>, 9> const expected_seq =
  {
    std::make_pair(boost::pre, 0),
    std::make_pair(boost::pre, 1),
    std::make_pair(boost::in, 1),
    std::make_pair(boost::post, 1),
    std::make_pair(boost::in, 0),
    std::make_pair(boost::pre, 2),
    std::make_pair(boost::in, 2),
    std::make_pair(boost::post, 2),
    std::make_pair(boost::post, 0)
  };
  tree_visitor<boost::visit, vertex_descriptor> visitor;
  depth_first_visit(tree, added[0], visitor, color);
  BOOST_CHECK(boost::equal(visitor.visited, expected_seq));

  remove_edge(added[0], added[1], tree);
  remove_edge(added[0], added[2], tree);
  BOOST_CHECK(!has_left_successor(added[0], tree));
  BOOST_CHECK(!has_right_successor(added[0], tree));
}

int test_main(int, char*[])
{
  BOOST_CONCEPT_ASSERT((boost::concepts::IncidenceGraph<boost::forward_binary_tree>));
  BOOST_CONCEPT_ASSERT((boost::concepts::BidirectionalGraph<boost::bidirectional_binary_tree>));
  // BOOST_CONCEPT_ASSERT((boost::concepts::VertexListGraph<boost::bidirectional_binary_tree>));
  // BOOST_CONCEPT_ASSERT((boost::concepts::MutableGraph<boost::bidirectional_binary_tree>));


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

  return 0;
}
