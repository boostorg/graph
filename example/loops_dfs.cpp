//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#include <boost/config.hpp>
#include <boost/concept/assert.hpp>
#include <iostream>
#include <fstream>
#include <stack>
#include <map>
#include <boost/lexical_cast.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/copy.hpp>
#include <boost/graph/reverse_graph.hpp>
#include "range_pair.hpp"

using namespace boost;

template <typename OutputIterator> 
class back_edge_recorder : public default_dfs_visitor
{
public:
  back_edge_recorder(OutputIterator out):m_out(out) { }

  template <typename Edge, typename Graph>
  void back_edge(Edge e, const Graph &)
  {
    *m_out++ = e;
  }
private:
  OutputIterator m_out;
};

// object generator function
template <typename OutputIterator>
back_edge_recorder<OutputIterator>
make_back_edge_recorder(OutputIterator out)
{
  return back_edge_recorder<OutputIterator> (out);
}

template <typename Graph, typename Loops> void
find_loops(typename graph_traits<Graph>::vertex_descriptor entry, 
           const Graph & g, 
           Loops & loops)    // A container of sets of vertices
{
  BOOST_CONCEPT_ASSERT(( BidirectionalGraphConcept<Graph> ));
  using Edge = typename graph_traits<Graph>::edge_descriptor;
  std::vector<Edge> back_edges;
  std::vector<default_color_type> color_map(num_vertices(g));
  depth_first_visit(g, entry,
                    make_back_edge_recorder(std::back_inserter(back_edges)),
                    make_iterator_property_map(color_map.begin(),
                                               get(vertex_index, g), color_map[0]));

  for (typename std::vector<Edge>::size_type i = 0; i < back_edges.size(); ++i) {
    typename Loops::value_type x;
    loops.emplace_back(x);
    compute_loop_extent(back_edges[i], g, loops.back());
  }
}

template <typename Graph, typename Set> void
compute_loop_extent(typename graph_traits <
                    Graph >::edge_descriptor back_edge, const Graph & g,
                    Set & loop_set)
{
  BOOST_CONCEPT_ASSERT(( BidirectionalGraphConcept<Graph> ));
  using Vertex = typename graph_traits<Graph>::vertex_descriptor;
  using Color = color_traits<default_color_type>;

  auto loop_tail = source(back_edge, g);
  auto loop_head = target(back_edge, g);

  std::vector<default_color_type>
    reachable_from_head(num_vertices(g), Color::white());
  default_color_type c;
  depth_first_visit(g, loop_head, default_dfs_visitor(),
                    make_iterator_property_map(reachable_from_head.begin(),
                                               get(vertex_index, g), c));

  std::vector<default_color_type> reachable_to_tail(num_vertices(g));
  reverse_graph<Graph> reverse_g(g);
  depth_first_visit(reverse_g, loop_tail, default_dfs_visitor(),
                    make_iterator_property_map(reachable_to_tail.begin(),
                                               get(vertex_index, g), c));

  for (const auto& vertex : make_range_pair(vertices(g)))
    if (reachable_from_head[vertex] != Color::white()
        && reachable_to_tail[vertex] != Color::white())
      loop_set.insert(vertex);
}


int
main(int argc, char *argv[])
{
  if (argc < 3) {
    std::cerr << "usage: loops_dfs <in-file> <out-file>" << std::endl;
    return -1;
  }
  GraphvizDigraph g_in;
  read_graphviz(argv[1], g_in);

  using Graph = adjacency_list < vecS, vecS, bidirectionalS,
    GraphvizVertexProperty,
    GraphvizEdgeProperty, GraphvizGraphProperty >;
  using Vertex = graph_traits<Graph>::vertex_descriptor;

  Graph g;
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  // VC++ has trouble with the get_property() function
  get_property(g, graph_name) = "loops";
#endif

  copy_graph(g_in, g);

  using set_t = std::set<Vertex>;
  using list_of_sets_t = std::list<set_t>;
  list_of_sets_t loops;
  auto entry = *vertices(g).first;

  find_loops(entry, g, loops);

  auto vattr_map = get(vertex_attribute, g);
  auto eattr_map = get(edge_attribute, g);

  for (const auto& loop : loops) {
    std::vector<bool> in_loop(num_vertices(g), false);
    for (const auto& vertex : loop) {
      vattr_map[vertex]["color"] = "gray";
      in_loop[vertex] = true;
    }
    for (const auto& edge : make_range_pair(edges(g)))
      if (in_loop[source(edge, g)] && in_loop[target(edge, g)])
        eattr_map[edge]["color"] = "gray";
  }

  std::ofstream loops_out(argv[2]);
#if defined(BOOST_MSVC) && BOOST_MSVC <= 1300
  // VC++ has trouble with the get_property() functions
  loops_out << "digraph loops {\n"
            << "size=\"3,3\"\n"
            << "ratio=\"fill\"\n"
            << "shape=\"box\"\n";
  for (const auto& vertex : make_range_pair(vertices(g))) {
    loops_out << vertex << "[";
    for (auto ai = vattr_map[vertex].begin();
         ai != vattr_map[vertex].end(); ++ai) {
      loops_out << ai->first << "=" << ai->second;
      if (next(ai) != vattr_map[vertex].end())
        loops_out << ", ";
    }
    loops_out<< "]";
  }

  for (const auto& edge : make_range_pair(edges(g))) {
    loops_out << source(edge, g) << " -> " << target(edge, g) << "[";
    auto& attr_map = eattr_map[edge];
    for (auto eai = attr_map.begin();
         eai != attr_map.end(); ++eai) {
      loops_out << eai->first << "=" << eai->second;
      if (next(eai) != attr_map.end())
        loops_out << ", ";
    }
    loops_out<< "]";
  }
  loops_out << "}\n";
#else
  get_property(g, graph_graph_attribute)["size"] = "3,3";
  get_property(g, graph_graph_attribute)["ratio"] = "fill";
  get_property(g, graph_vertex_attribute)["shape"] = "box";

  write_graphviz(loops_out, g,
                 make_vertex_attributes_writer(g),
                 make_edge_attributes_writer(g),
                 make_graph_attributes_writer(g));
#endif
  return 0;
}
