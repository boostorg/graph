#include <iostream>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graph_utility.hpp>

using namespace boost;

/*
  Sample output:

  original graph:
  0 <--> 3 3 2 
  1 <--> 3 
  2 <--> 0 3 
  3 <--> 0 0 1 2 
  1(0,3) 2(0,3) 3(1,3) 4(2,0) 5(3,2) 

  removing edges connecting 0 and 3
  0 <--> 2 
  1 <--> 3 
  2 <--> 0 3 
  3 <--> 1 2 
  3(1,3) 4(2,0) 5(3,2) 
  removing edges with weight greater than 3
  0 <--> 
  1 <--> 3 
  2 <--> 
  3 <--> 1 
  3(1,3) 
  
 */

typedef adjacency_list<vecS, vecS, undirectedS, 
  no_property, property<edge_weight_t, int> > Graph;

struct incident_to {
  incident_to(graph_traits<Graph>::vertex_descriptor v_,
	     Graph& g_) : v(v_), g(g_) { }
  bool operator()(graph_traits<Graph>::edge_descriptor e) {
    graph_traits<Graph>::vertex_descriptor a, b;
    tie(a, b) = incident(e, g);
    return a == v || b == v;
  }
  graph_traits<Graph>::vertex_descriptor v;
  Graph& g;
};

struct has_weight_greater_than {
  has_weight_greater_than(int w_, Graph& g_) : w(w_), g(g_) { }
  bool operator()(graph_traits<Graph>::edge_descriptor e) {
#ifdef BOOST_MSVC
    return get(get(edge_weight, g), e) > w;
#else
    // This version of get breaks VC++
    return get(edge_weight, g, e) > w;
#endif
  }
  int w;
  Graph& g;
};

int
main()
{
  typedef std::pair<std::size_t,std::size_t> Edge;
  Edge edges[5] = { Edge(0, 3), Edge(0, 3),
		    Edge(1, 3),
		    Edge(2, 0),
		    Edge(3, 2) };

  Graph g(4, edges, edges + 5);
  property_map<Graph, edge_weight_t>::type 
    weight = get(edge_weight, g);

  int w = 0;
  graph_traits<Graph>::edge_iterator ei, ei_end;
  for (tie(ei, ei_end) = boost::edges(g); ei != ei_end; ++ei)
    weight[*ei] = ++w;

  std::cout << "original graph:" << std::endl;
  print_graph(g, get(vertex_index, g));
  print_edges2(g, get(vertex_index, g), get(edge_weight, g));
  std::cout << std::endl;

  std::cout << "removing edges connecting 0 and 3" << std::endl;
  remove_out_edge_if(vertex(0,g), incident_to(vertex(3,g), g), g);
  print_graph(g, get(vertex_index, g));
  print_edges2(g, get(vertex_index, g), get(edge_weight, g));

  std::cout << "removing edges with weight greater than 3" << std::endl;
  remove_edge_if(has_weight_greater_than(3, g), g);
  print_graph(g, get(vertex_index, g));
  print_edges2(g, get(vertex_index, g), get(edge_weight, g));

  return 0;
}
