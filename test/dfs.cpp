
#include <boost/config.hpp>
#include <boost/test_framework.hpp>
#include <stdlib.h>

#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_archetypes.hpp>
#include "generate_random_graph.hpp"

template <typename ColorMap, typename ParentMap,
  typename DiscoverTimeMap, typename FinishTimeMap>
class dfs_test_visitor {
  typedef typename boost::property_traits<ColorMap>::value_type ColorValue;
  typedef typename boost::color_traits<ColorValue> Color;
public:
  dfs_test_visitor(ColorMap color, ParentMap p, DiscoverTimeMap d,
                   FinishTimeMap f)
    : m_color(color), m_parent(p), 
    m_discover_time(d), m_finish_time(f), m_time(0) { }
  
  template <class Vertex, class Graph>
  void initialize_vertex(Vertex u, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, u) == Color::white() );
  }
  template <class Vertex, class Graph>
  void start_vertex(Vertex u, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, u) == Color::white() );
  }
  template <class Vertex, class Graph>
  void discover_vertex(Vertex u, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, u) == Color::gray() );
    BOOST_TEST_VERIFY( get(m_color, get(m_parent, u)) == Color::gray() );

    put(m_discover_time, u, m_time++);
  }
  template <class Edge, class Graph>
  void examine_edge(Edge e, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, source(e, g)) == Color::gray() );
  }
  template <class Edge, class Graph>
  void tree_edge(Edge e, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, target(e, g)) == Color::white() );

    put(m_parent, target(e, g), source(e, g));
  }
  template <class Edge, class Graph>
  void back_edge(Edge e, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, target(e, g)) == Color::gray() );
  }
  template <class Edge, class Graph>
  void forward_or_cross_edge(Edge e, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, target(e, g)) == Color::black() );
  }
  template <class Vertex, class Graph>
  void finish_vertex(Vertex u, Graph& g) {
    BOOST_TEST_VERIFY( get(m_color, u) == Color::black() );
    
    put(m_finish_time, u, m_time++);
  }
private:
  ColorMap m_color;
  ParentMap m_parent;
  DiscoverTimeMap m_discover_time;
  FinishTimeMap m_finish_time;
  typename boost::property_traits<DiscoverTimeMap>::value_type m_time;
};


// usage: dfs.exe [max-vertices=15]

int test_main(int argc, char* argv[])
{
  if (0) { // compile only
    // Check boost::depth_first_search() version 1 requirements with archetypes
    typedef boost::default_constructible_archetype<
      boost::sgi_assignable_archetype< 
      boost::equality_comparable_archetype<> > > Vertex;
    typedef boost::vertex_list_graph_archetype<Vertex, boost::directed_tag, 
      boost::allow_parallel_edge_tag > VLGraph;
    typedef boost::property_graph_archetype<VLGraph, boost::vertex_color_t, 
      boost::color_value_archetype> PGraph;
    PGraph g_arch1;
    boost::depth_first_search(g_arch1, boost::dfs_visitor<>());

    // Check boost::depth_first_search() version 2 requirements with archetypes
    VLGraph g_arch2;
    typedef boost::read_write_property_map_archetype<Vertex,
      boost::color_value_archetype > color_map;
    boost::depth_first_search(g_arch2, boost::dfs_visitor<>(), color_map());
  }
  
  typedef boost::adjacency_list<boost::vecS, boost::vecS, boost::directedS,
    boost::property<boost::vertex_color_t, boost::default_color_type> > Graph;
  typedef boost::graph_traits<Graph>::vertex_descriptor vertex_descriptor;
  typedef boost::property_map<Graph, boost::vertex_color_t>::type ColorMap;
  typedef boost::property_traits<ColorMap>::value_type ColorValue;
  typedef boost::color_traits<ColorValue> Color;

  boost::graph_traits<Graph>::vertices_size_type max_V = 15;
  boost::graph_traits<Graph>::vertices_size_type i, k;
  boost::graph_traits<Graph>::edges_size_type j;
  
  boost::graph_traits<Graph>::vertex_iterator vi, vi_end, ui, ui_end;

  if (argc > 1)
    max_V = atoi(argv[1]);

  for (i = 0; i < max_V; ++i)
    for (j = 0; j < i*i; ++j) {
      Graph g;
      generate_random_graph(g, i, j);

      ColorMap color = get(boost::vertex_color, g);
      std::vector<vertex_descriptor> parent(num_vertices(g));
      for (k = 0; k < num_vertices(g); ++k)
        parent[k] = k;
      std::vector<int> discover_time(num_vertices(g)),
        finish_time(num_vertices(g));

      dfs_test_visitor<ColorMap, vertex_descriptor*,
        int*, int*> visitor(color, &parent[0], 
                            &discover_time[0], &finish_time[0]);

      boost::depth_first_search(g, visitor);

      // all vertices should be black
      for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
        BOOST_TEST_VERIFY(get(color, *vi) == Color::black());
      
      // check parenthesis structure of discover/finish times
      // See CLR p.480
      for (boost::tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui)
        for (boost::tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi) {
          vertex_descriptor u = *ui, v = *vi;
          if (u != v) {
            BOOST_TEST_VERIFY( finish_time[u] < discover_time[v]
                               || finish_time[v] < discover_time[u]
                               || (discover_time[v] < discover_time[u]
                                   && finish_time[u] < finish_time[v]
                                   && boost::is_descendant(u, v, &parent[0]))
                               || (discover_time[u] < discover_time[v]
                                   && finish_time[v] < finish_time[u]
                                   && boost::is_descendant(v, u, &parent[0]))
                               );
          }
        }
    }

  return 0;
}

