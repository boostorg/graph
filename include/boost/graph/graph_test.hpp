#ifndef BOOST_GRAPH_TEST_HPP
#define BOOST_GRAPH_TEST_HPP

#include <boost/test/test_tools.hpp>
#include <boost/graph/iteration_macros.hpp>

namespace boost {

  template <typename Graph>
  struct graph_test
  {
  
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;
    typedef typename graph_traits<Graph>::edge_descriptor edge_t;
    typedef typename graph_traits<Graph>::vertices_size_t v_size_t;
    typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iter;

    struct ignore_vertex {
      ignore_vertex(vertex_t v) : v(v) { }
      bool operator()(vertex_t x) { return x != v; }
      vertex_t v;
    };
    struct ignore_edge {
      ignore_edge(edge_t e) : e(e) { }
      bool operator()(edge_t x) { return x != e; }
      edge_t e;
    };

    // PRE: g and orig are isomorphic
    void test_add_vertex(const Graph& orig)
    {
      Graph g;
      std::vector<vertex_t> iso_map;
      copy_graph(orig, g, iso_map);

      vertex_t v = add_vertex(g);
      
      BOOST_TEST(num_vertices(g) == num_vertices(orig) + 1);

      BOOST_TEST(out_degree(v, g) == 0);

      // Make sure the rest of the graph stayed the same
      BOOST_TEST((verify_isomorphism
		  (make_filtered_graph(g, keep_all(), ignore_vertex(v)),
		   orig, iso_map)));
    }
    
    void test_add_edge(vertex_t u, vertex_t v, const Graph& orig)
    {
      Graph g;
      std::vector<vertex_t> iso_map;
      copy_graph(orig, g, 
		 make_iterator_property_map(iso_map, get(vertex_index, g)));
      
      bool parallel_edge_exists	= contains(adjacent_vertices(u, g), v);
      
      std::pair<edge_t, bool> p = add_edge(u, v, g);
      edge_t e = p.first;
      bool added = p.second;

      if (is_undirected(g) && u == v) // self edge
	BOOST_TEST(added == false);
      else if (parallel_edge_exists)
        BOOST_TEST(allows_parallel_edges(g) && added == true
		   || !allows_parallel_edges(g) && added == false);
      else
	BOOST_TEST(added == true);

      if (p.second == true) { // edge added
	BOOST_TEST(num_edges(g) == num_edge(orig) + 1);
	
	BOOST_TEST(contains(out_edge(u, g), e) == true);
	
	BOOST_TEST((verify_isomorphism
		    (make_filtered_graph(g, ignore_edge(e)),
		     orig, iso_map)));
      }
      else { // edge not added

	if (! (is_undirected(g) && u == v)) {
	  // e should be a parallel edge
	  BOOST_TEST(source(e, g) == u);
	  BOOST_TEST(target(e, g) == v);
	}

	// The graph should not be changed.
	BOOST_TEST((verify_isomorphism(g, orig, iso_map)));	
      }
    } // test_add_edge()


  };
  
}// namespace boost

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_TEST_HPP
