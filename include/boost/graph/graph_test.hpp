#ifndef BOOST_GRAPH_TEST_HPP
#define BOOST_GRAPH_TEST_HPP

#include <boost/test/test_tools.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/isomorphism.hpp>
#include <boost/graph/copy.hpp>

namespace boost {

  template <typename Graph>
  struct graph_test
  {
  
    typedef typename graph_traits<Graph>::vertex_descriptor vertex_t;
    typedef typename graph_traits<Graph>::edge_descriptor edge_t;
    typedef typename graph_traits<Graph>::vertices_size_type v_size_t;
    typedef typename graph_traits<Graph>::out_edge_iterator out_edge_iter;
    typedef typename property_map<Graph, vertex_index_t>::const_type index_map_t;
    typedef iterator_property_map<typename std::vector<vertex_t>::iterator,index_map_t,vertex_t,vertex_t&> IsoMap;

    struct ignore_vertex {
      ignore_vertex() { }
      ignore_vertex(vertex_t v) : v(v) { }
      bool operator()(vertex_t x) const { return x != v; }
      vertex_t v;
    };
    struct ignore_edge {
      ignore_edge() { }
      ignore_edge(edge_t e) : e(e) { }
      bool operator()(edge_t x) const { return x != e; }
      edge_t e;
    };

    // PRE: g and orig are isomorphic
    void test_add_vertex(const Graph& orig)
    {
      Graph g;
      std::vector<vertex_t> iso_vec(num_vertices(orig));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, orig));
      copy_graph(orig, g, orig_to_copy(iso_map));

      assert((verify_isomorphism(orig, g, iso_map)));

      vertex_t v = add_vertex(g);
      
      BOOST_TEST(num_vertices(g) == num_vertices(orig) + 1);

      BOOST_TEST(out_degree(v, g) == 0);

      // Make sure the rest of the graph stayed the same
      BOOST_TEST((verify_isomorphism
		  (orig, make_filtered_graph(g, keep_all(), ignore_vertex(v)),
		   iso_map)));
    }
    
    void test_add_edge(vertex_t u, vertex_t v, const Graph& orig)
    {
      Graph g;
      std::vector<vertex_t> iso_vec(num_vertices(orig));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, orig));
      copy_graph(orig, g, orig_to_copy(iso_map));
      
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
	BOOST_TEST(num_edges(g) == num_edges(orig) + 1);
	
	BOOST_TEST(contains(out_edges(u, g), e) == true);
	
	BOOST_TEST((verify_isomorphism
		    (orig, make_filtered_graph(g, ignore_edge(e)), iso_map)));
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
