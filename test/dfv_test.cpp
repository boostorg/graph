//=======================================================================
// Copyright 2022 Ralf Kohrt

// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/core/lightweight_test.hpp>

using namespace boost;

// Set up the vertex names
enum vertex_id_t { u, v, w, x, y, z, N };


struct counting_dfs_visitor
{
  counting_dfs_visitor():
    vertex_events(0),
    seen_edges(0)
  {}

  template<typename Vertex, typename Graph>
  void initialize_vertex(Vertex v, const Graph& g)
  {
    ++vertex_events;
  }

  template<typename Vertex, typename Graph>
  void start_vertex(Vertex v, const Graph& g)
  {
    ++vertex_events;
  }

  template<typename Vertex, typename Graph>
  void discover_vertex(Vertex v, const Graph& g)
  {
    ++vertex_events;
  }

  template<typename Edge, typename Graph>
  void examine_edge(Edge e, const Graph& g)
  {
    ++seen_edges;
  }

  template<typename Edge, typename Graph>
  void tree_edge(Edge e, const Graph& g)
  {}

  template<typename Edge, typename Graph>
  void back_edge(Edge e, const Graph& g)
  {}

  template<typename Edge, typename Graph>
  void forward_or_cross_edge(Edge e, const Graph& g)
  {}

  template<typename Vertex, typename Graph>
  void finish_vertex(Vertex v, const Graph& g)
  {
    ++vertex_events;
  }

  size_t vertex_events;
  size_t seen_edges;

};

void
test_dfv_returns_copied_visitor()
{
  typedef adjacency_list<listS,
                         vecS,
                         undirectedS,
                         // Vertex properties
                         property<vertex_color_t, default_color_type> >
    Graph;
  typedef typename boost::property_map< Graph, boost::vertex_color_t >::type
    ColorMap;

  // Specify the edges in the graph
  typedef std::pair<int, int> E;
  E edge_array[] = { E(u, v), E(u, w), E(u, x), E(x, v), E(y, x),
                     E(v, y), E(w, y), E(w, z), E(z, z) };
  Graph g(edge_array, edge_array + sizeof(edge_array) / sizeof(E), N);

  ColorMap color = get(boost::vertex_color, g);
  counting_dfs_visitor visitor_copy = depth_first_visit(g, vertex(u, g), counting_dfs_visitor(), color);
  BOOST_TEST(visitor_copy.vertex_events == num_vertices(g)*2u + 1u); // discover_vertex + finish_vertex for each vertex and once start_vertex
  BOOST_TEST(visitor_copy.seen_edges == num_edges(g)*2u);
}

int
main(int argc, char* argv[])
{
  test_dfv_returns_copied_visitor();
  return boost::report_errors();
}
