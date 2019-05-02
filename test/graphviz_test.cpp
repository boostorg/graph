// Copyright 2004-5 Trustees of Indiana University

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//
// graphviz_test.cpp - Test cases for the Boost.Spirit implementation of a
// Graphviz DOT Language reader.
//

// Author: Ronald Garcia

#define BOOST_GRAPHVIZ_USE_ISTREAM
#define BOOST_TEST_MODULE TestGraphviz
#include <boost/regex.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/assign/std/map.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/compressed_sparse_row_graph.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/property_map/dynamic_property_map.hpp>
#include <algorithm>
#include <string>
#include <iostream>
#include <iterator>
#include <map>
#include <utility>
#include <boost/test/test_tools.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/floating_point_comparison.hpp>
//#include <boost/core/lightweight_test.hpp>


using namespace std;
using namespace boost;

using namespace boost::assign;

typedef std::string node_t;

typedef std::pair<node_t,node_t> edge_t;

typedef std::map<node_t,float> mass_map_t;

typedef std::map<edge_t,double> weight_map_t;

typedef istringstream gs_t;

typedef property <
  vertex_name_t,
  std::string,
  property <
    vertex_color_t,
    float > > vertex_p;


typedef property <
  edge_weight_t,
  double > edge_p;


typedef property <
  graph_name_t,
  std::string > graph_p;


struct
vertex_p_bundled {
  std::string name;
  float color;
};


struct
edge_p_bundled {
  double weight;
};


template <
  typename graph_t,
  typename NameMap,
  typename MassMap,
  typename WeightMap>
void
test_graph_full(
  std::istream& dotfile,
  graph_t& graph,
  std::size_t correct_num_vertices,
  mass_map_t const& masses,
  weight_map_t const& weights,
  std::string const& node_id,
  std::string const& g_name,
  NameMap name,
  MassMap mass,
  WeightMap weight
){

  // Construct a graph and set up the dynamic_property_maps.
  dynamic_properties dp(ignore_other_properties);
  dp.property(node_id,name);
  dp.property("mass",mass);
  dp.property("weight",weight);

  boost::ref_property_map<graph_t*,std::string> gname(
    get_property(graph,graph_name));
  dp.property("name",gname);

  std::string data;
  dotfile >> std::noskipws;
  std::copy(std::istream_iterator<char>(dotfile),
            std::istream_iterator<char>(),
            std::back_inserter(data));
  BOOST_TEST(read_graphviz(data.begin(),data.end(),graph,dp,node_id));

  // check correct vertex count
  BOOST_TEST(num_vertices(graph) == correct_num_vertices);
  // check masses
  if(!masses.empty()) {
    // assume that all the masses have been set
    // for each vertex:
    typename graph_traits<graph_t>::vertex_iterator i,j;
    for(boost::tie(i,j) = vertices(graph); i != j; ++i) {
      //  - get its name
      auto node_name = get(name,*i);
      //  - get its mass
      bool eq_check = masses.find(node_name) != masses.end();
      BOOST_TEST(eq_check);
      float ref_mass = masses.find(node_name)->second;
      //  - compare the mass to the result in the table
      BOOST_CHECK_CLOSE(get(mass,*i), ref_mass, 0.01f);
    }
  }
  // check weights
  if(!weights.empty()) {
    // assume that all weights have been set
    /// for each edge:
    typename graph_traits<graph_t>::edge_iterator i,j;
    for(boost::tie(i,j) = edges(graph); i != j; ++i) {
      //  - get its name
      auto edge_name = make_pair(get(name, source(*i,graph)), get(name, target(*i,graph)));
      // - get its weight
      double edge_weight = get(weight,*i);
      bool eq_check = weights.find(edge_name) != weights.end();
      BOOST_TEST(eq_check);
      double ref_weight = weights.find(edge_name)->second;
      // - compare the weight to teh result in the table
      BOOST_CHECK_CLOSE(edge_weight, ref_weight, 0.01);
    }
  }
  if(!g_name.empty()) {
    std::string parsed_name = get_property(graph,graph_name);
    BOOST_TEST(parsed_name == g_name);
  }
}



template <
  typename graph_t>
void
test_graph_short(
  std::istream& dotfile,
  std::size_t correct_num_vertices,
  mass_map_t const& masses,
  weight_map_t const& weights,
  std::string const& node_id = "node_id",
  std::string const& g_name = std::string()
){

  graph_t g;

  test_graph_full(
    dotfile,
    g,
    correct_num_vertices,
    masses,
    weights,
    node_id,
    g_name,
    get(vertex_name, g),
    get(vertex_color, g),
    get(edge_weight, g));
}


// Basic directed graph tests
void basic_directed_graph_1(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",7.7f) ("e", 6.66f);
  gs_t gs("digraph { a  node [mass = 7.7] c e [mass = 6.66] }");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t());
}


void
basic_directed_graph_2(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("e", 6.66f);
  gs_t gs("digraph { a  node [mass = 7.7] \"a\" e [mass = 6.66] }");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,2,masses,weight_map_t());
}


void
basic_directed_graph_3(
){
  weight_map_t weights;
  insert( weights )(make_pair("a","b"),0.0)
    (make_pair("c","d"),7.7)(make_pair("e","f"),6.66)
    (make_pair("d","e"),0.5)(make_pair("e","a"),0.5);
  gs_t gs("digraph { a -> b eDge [weight = 7.7] "
          "c -> d e-> f [weight = 6.66] "
          "d ->e->a [weight=.5]}");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,6,mass_map_t(),weights);
}


// undirected graph with alternate node_id property name
void
undirected_graph_alternate_node_id(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",7.7f) ("e", 6.66f);
  gs_t gs("graph { a  node [mass = 7.7] c e [mass = 6.66] }");
  typedef adjacency_list < vecS, vecS, undirectedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t(), "nodenames");
}


// Basic undirected graph tests
void
basic_undirected_graph_1(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",7.7f) ("e", 6.66f);
  gs_t gs("graph { a  node [mass = 7.7] c e [mass =\\\n6.66] }");
  typedef adjacency_list < vecS, vecS, undirectedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t());
}


void
basic_undirected_graph_2(
){
  weight_map_t weights;
  insert( weights )(make_pair("a","b"),0.0)
    (make_pair("c","d"),7.7)(make_pair("e","f"),6.66);
  gs_t gs("graph { a -- b eDge [weight = 7.7] "
          "c -- d e -- f [weight = 6.66] }");
  typedef adjacency_list < vecS, vecS, undirectedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,6,mass_map_t(),weights);
}


// Mismatch directed graph test
void
mismatch_directed_graph(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",7.7f) ("e", 6.66f);
  gs_t gs("graph { a  nodE [mass = 7.7] c e [mass = 6.66] }");
  try {
    typedef adjacency_list < vecS, vecS, directedS,
      vertex_p, edge_p, graph_p > graph_t;
    test_graph_short<graph_t>(gs,3,masses,weight_map_t());
    BOOST_ERROR("Failed to throw boost::undirected_graph_error.");
  } catch (boost::undirected_graph_error&) {
  } catch (boost::directed_graph_error&) {
    BOOST_ERROR("Threw boost::directed_graph_error, should have thrown boost::undirected_graph_error.");
  }
}


// Mismatch undirected graph test
void
mismatch_undirected_graph(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",7.7f) ("e", 6.66f);
  gs_t gs("digraph { a  node [mass = 7.7] c e [mass = 6.66] }");
  try {
    typedef adjacency_list < vecS, vecS, undirectedS,
      vertex_p, edge_p, graph_p > graph_t;
    test_graph_short<graph_t>(gs,3,masses,weight_map_t());
    BOOST_ERROR("Failed to throw boost::directed_graph_error.");
  } catch (boost::directed_graph_error&) {}
}


// Complain about parallel edges
void
complain_about_parallel_edges(
){
  BOOST_TEST_CHECKPOINT("Complain about parallel edges");
  weight_map_t weights;
  insert( weights )(make_pair("a","b"),7.7);
  gs_t gs("diGraph { a -> b [weight = 7.7]  a -> b [weight = 7.7] }");
  try {
    typedef adjacency_list < setS, vecS, directedS,
      vertex_p, edge_p, graph_p > graph_t;
    test_graph_short<graph_t>(gs,2,mass_map_t(),weights);
    BOOST_ERROR("Failed to throw boost::bad_parallel_edge.");
  } catch (boost::bad_parallel_edge&) {}
}


// Handle parallel edges gracefully
void
handle_parallel_edges_gracefully(
){
  weight_map_t weights;
  insert( weights )(make_pair("a","b"),7.7);
  gs_t gs("digraph { a -> b [weight = 7.7]  a -> b [weight = 7.7] }");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,2,mass_map_t(),weights);
}


// Graph Property Test 1
void
graph_property_test_1(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",0.0f) ("e", 6.66f);
  gs_t gs("digraph { graph [name=\"foo \\\"escaped\\\"\"]  a  c e [mass = 6.66] }");
  std::string graph_name("foo \"escaped\"");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t(),"", graph_name);
}


// Graph Property Test 2
void
graph_property_test_2(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",0.0f) ("e", 6.66f);
  gs_t gs("digraph { name=\"fo\"+ \"\\\no\"  a  c e [mass = 6.66] }");
  std::string graph_name("foo");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t(),"", graph_name);
}


// Graph Property Test 3 (HTML)
void
graph_property_test_3(
){
  mass_map_t masses;
  insert ( masses )  ("a",0.0f) ("c",0.0f) ("e", 6.66f);
  std::string graph_name = "<html title=\"x'\" title2='y\"'>foo<b><![CDATA[><bad tag&>]]>bar</b>\n<br/>\nbaz</html>";
  gs_t gs("digraph { name=" + graph_name + "  a  c e [mass = 6.66] }");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,3,masses,weight_map_t(),"", graph_name);
}


// Comments embedded in strings
void
comments_embedded_in_strings(
){
  gs_t gs(
    "digraph { "
    "a0 [ label = \"//depot/path/to/file_14#4\" ];"
    "a1 [ label = \"//depot/path/to/file_29#9\" ];"
    "a0 -> a1 [ color=gray ];"
    "}");
  typedef adjacency_list < vecS, vecS, directedS,
    vertex_p, edge_p, graph_p > graph_t;
  test_graph_short<graph_t>(gs,2,mass_map_t(),weight_map_t());
}


// Currently broken
// BOOST_AUTO_TEST_CASE (basic_csr_directed_graph) {
//   weight_map_t weights;
//   insert( weights )(make_pair("a","b"),0.0)
//     (make_pair("c","d"),7.7)(make_pair("e","f"),6.66)
//     (make_pair("d","e"),0.5)(make_pair("e","a"),0.5);
//   gs_t gs("digraph { a -> b eDge [weight = 7.7] "
//           "c -> d e-> f [weight = 6.66] "
//           "d ->e->a [weight=.5]}");
//   typedef compressed_sparse_row_graph<directedS, vertex_p_bundled, edge_p_bundled, graph_p > graph_t;
//   BOOST_TEST((test_graph<graph_t>(gs,6,mass_map_t(),weights,"node_id","",&vertex_p_bundled::name,&vertex_p_bundled::color,&edge_p_bundled::weight)));
// }


// void basic_csr_directed_graph_ext_props(
// ){
//   weight_map_t weights;
//   insert( weights )(make_pair("a","b"),0.0)
//     (make_pair("c","d"),7.7)(make_pair("e","f"),6.66)
//     (make_pair("d","e"),0.5)(make_pair("e","a"),0.5);

//   gs_t gs("digraph { a -> b eDge [weight = 7.7] "
//           "c -> d e-> f [weight = 6.66] "
//           "d ->e->a [weight=.5]}");

//   typedef compressed_sparse_row_graph<
//     directedS,
//     no_property,
//     no_property,
//     graph_p >
//   graph_t;

//   graph_t g;

//   test_graph_full(
//     gs,
//     g,
//     6,
//     mass_map_t(),
//     weights,
//     "node_id",
//     "",
//     get(vertex_index, g),
//     get(vertex_index, g),
//     get(edge_index, g));
// }


int
main(
){

  basic_directed_graph_1();

  basic_directed_graph_2();

  basic_directed_graph_3();

  undirected_graph_alternate_node_id();

  basic_undirected_graph_1();

  basic_undirected_graph_2();

  mismatch_undirected_graph();

  complain_about_parallel_edges();

  handle_parallel_edges_gracefully();

  graph_property_test_1();

  graph_property_test_2();

  graph_property_test_3();

  comments_embedded_in_strings();

  // basic_csr_directed_graph_ext_props();

  return 0;
}