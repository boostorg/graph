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
    typedef typename graph_traits<Graph>::degree_size_type deg_size_t;
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
    struct ignore_edges {
      ignore_edges() { }
      ignore_edges(vertex_t s, vertex_t t, const Graph& g) 
	: s(s), t(t), g(g) { }
      bool operator()(edge_t x) const { 
	return !(source(x, g) == s && target(x, g) == t);
      }
      vertex_t s; vertex_t t; const Graph& g;
    };

    void test_add_vertex(Graph& g)
    {
      Graph cpy;
      std::vector<vertex_t> iso_vec(num_vertices(g));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, g));
      copy_graph(g, cpy, orig_to_copy(iso_map));

      assert((verify_isomorphism(g, cpy, iso_map)));

      vertex_t v = add_vertex(g);
      
      BOOST_TEST(num_vertices(g) == num_vertices(cpy) + 1);

      BOOST_TEST(out_degree(v, g) == 0);

      // Make sure the rest of the graph stayed the same
      BOOST_TEST((verify_isomorphism
		  (make_filtered_graph(g, keep_all(), ignore_vertex(v)), cpy,
		   iso_map)));
    }
    
    void test_add_edge(vertex_t u, vertex_t v, Graph& g)
    {
      Graph cpy;
      std::vector<vertex_t> iso_vec(num_vertices(g));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, g));
      copy_graph(g, cpy, orig_to_copy(iso_map));

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
	BOOST_TEST(num_edges(g) == num_edges(cpy) + 1);
	
	BOOST_TEST(contains(out_edges(u, g), e) == true);
	
	BOOST_TEST((verify_isomorphism
		    (make_filtered_graph(g, ignore_edge(e)), cpy, iso_map)));
      }
      else { // edge not added

	if (! (is_undirected(g) && u == v)) {
	  // e should be a parallel edge
	  BOOST_TEST(source(e, g) == u);
	  BOOST_TEST(target(e, g) == v);
	}

	// The graph should not be changed.
	BOOST_TEST((verify_isomorphism(g, cpy, iso_map)));
      }
    } // test_add_edge()


    void test_remove_edge(vertex_t u, vertex_t v, Graph& g)
    {
      Graph cpy;
      std::vector<vertex_t> iso_vec(num_vertices(g));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, g));
      copy_graph(g, cpy, orig_to_copy(iso_map));

      deg_size_t occurances = count(adjacent_vertices(u, g), v);

      remove_edge(u, v, g);
      
      BOOST_TEST(num_edges(g) + occurances == num_edges(cpy));

      BOOST_TEST((verify_isomorphism
		  (g, make_filtered_graph(cpy, ignore_edges(u,v,cpy)),
		   iso_map)));
    }

    void test_remove_edge(edge_t e, Graph& g)
    {
      Graph cpy;
      std::vector<vertex_t> iso_vec(num_vertices(g));
      IsoMap iso_map(iso_vec.begin(), get(vertex_index, g));
      copy_graph(g, cpy, orig_to_copy(iso_map));

      vertex_t u = source(e, g), v = target(e, g);
      deg_size_t occurances = count(adjacent_vertices(u, g), v);
      
      remove_edge(e, g);

      BOOST_TEST(num_edges(g) + 1 == num_edges(cpy));
      BOOST_TEST(count(adjacent_vertices(u, g), v) + 1 == occurances);

      BOOST_TEST((verify_isomorphism
		  (g, make_filtered_graph(cpy, ignore_edge(e)),
		   iso_map)));
    }

  };
  
}// namespace boost

#include <boost/graph/iteration_macros_undef.hpp>

#endif // BOOST_GRAPH_TEST_HPP
