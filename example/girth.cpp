
/*
  Adapted from the GIRTH program of the Stanford GraphBase.

  Sample output:

  This program explores the girth and diameter of Ramanujan graphs.
  The bipartite graphs have q^3-q vertices, and the non-bipartite
  graphs have half that number. Each vertex has degree p+1.
  Both p and q should be odd prime numbers;
    or you can try p = 2 with q = 17 or 43.

  Choose a branching factor, p: 2
  Ok, now choose the cube root of graph size, q: 17
  Starting at any given vertex, there are
  3 vertices at distance 1,
  6 vertices at distance 2,
  12 vertices at distance 3,
  24 vertices at distance 4,
  46 vertices at distance 5,
  90 vertices at distance 6,
  169 vertices at distance 7,
  290 vertices at distance 8,
  497 vertices at distance 9,
  634 vertices at distance 10,
  521 vertices at distance 11,
  138 vertices at distance 12,
  13 vertices at distance 13,
  3 vertices at distance 14,
  1 vertices at distance 15.
  So the diameter is 15, and the girth is 9.
  
 */

#include <boost/config.hpp>
#include <vector>
#include <list>
#include <iostream>
#include <boost/pending/limits.hpp>
#include <boost/graph/stanford_graph.hpp>
#include <boost/graph/breadth_first_search.hpp>
#include <boost/graph/graph_utility.hpp>

typedef boost::graph_traits<Graph*> Traits;
typedef Traits::vertex_descriptor vertex_descriptor;
typedef Traits::edge_descriptor edge_descriptor;
typedef Traits::vertex_iterator vertex_iterator;

std::vector<std::size_t> distance_list;

typedef boost::v_property<long> dist_t;
boost::property_map<Graph*, dist_t>::type distance;

typedef boost::u_property<vertex_descriptor> pred_t;
boost::property_map<Graph*, pred_t>::type predecessor;

typedef boost::w_property<long> color_t;
boost::property_map<Graph*, color_t>::type color;

namespace boost {
  long white(long) { return 0; } // "unseen"
  long gray(long) { return 2; }  // "seen"
  long black(long) { return 1; } // "known"
}

class diameter_and_girth_visitor : public boost::bfs_visitor<>
{
public:
  diameter_and_girth_visitor(std::size_t& k_, std::size_t& girth_)
    : k(k_), girth(girth_) { }

  void tree_edge(edge_descriptor e, Graph* g) {
    vertex_descriptor u = source(e, g), v = target(e, g);
    k = distance[u] + 1;
    distance[v] = k;
    ++distance_list[k];
    predecessor[v] = u;
  }
  void cycle_edge(edge_descriptor e, Graph* g) {
    vertex_descriptor u = source(e, g), v = target(e, g);
    k = distance[u] + 1;
    if (distance[v] + k < girth && v != predecessor[u])
      girth = distance[v]+ k;
  }
private:
  std::size_t& k;
  std::size_t& girth;
};


int
main()
{
  std::cout <<
    "This program explores the girth and diameter of Ramanujan graphs." 
	    << std::endl;
  std::cout <<
    "The bipartite graphs have q^3-q vertices, and the non-bipartite" 
	    << std::endl;
  std::cout << 
    "graphs have half that number. Each vertex has degree p+1." 
	    << std::endl;
  std::cout << "Both p and q should be odd prime numbers;" << std::endl;
  std::cout << "  or you can try p = 2 with q = 17 or 43." << std::endl;

  while (1) {

    std::cout << std::endl
	      << "Choose a branching factor, p: ";
    long p = 0, q = 0;
    std::cin >> p;
    if (p == 0)
      break;
    std::cout << "Ok, now choose the cube root of graph size, q: ";
    std::cin >> q;
    if (q == 0)
      break;

    Graph* g;
    g = raman(p, q, 0L, 0L);
    if (g == 0) {
      std::cerr << " Sorry, I couldn't make that graph (error code "
	<< panic_code << ")" << std::endl;
      continue;
    }
    distance_list.clear();
    distance_list.resize(boost::num_vertices(g), 0);

    // obtain property maps
    distance = get(dist_t(), g);
    predecessor = get(pred_t(), g);
    color = get(color_t(), g);

    vertex_iterator i, end;
    for (boost::tie(i, end) = boost::vertices(g); i != end; ++i)
      distance[*i] = 0;

    std::size_t k = 0;
    std::size_t girth = std::numeric_limits<std::size_t>::max();
    diameter_and_girth_visitor visitor(k, girth);

    vertex_descriptor s = *boost::vertices(g).first;

    // Call Variant 2 of BFS
    boost::breadth_first_search(g, s, visitor, color);

    std::cout << "Starting at any given vertex, there are" << std::endl;

    for (long d = 1; distance_list[d] != 0; ++d)
      std::cout << distance_list[d] << " vertices at distance " << d
		<< (distance_list[d+1] != 0 ? "," : ".") << std::endl;

    std::cout << "So the diameter is " << k - 1
	      << ", and the girth is " << girth
	      << "." << std::endl;
  } // end while

  return 0;
}
