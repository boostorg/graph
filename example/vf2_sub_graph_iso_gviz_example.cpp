//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
using namespace std;

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/graphviz.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


// Define a print_callback
template <typename Graph1,
          typename Graph2,
          typename PropertyMap1,
          typename PropertyMap2>
struct print_callback {
  
  print_callback(const Graph1& graph1, const Graph2& graph2, 
                 PropertyMap1 p_map1, PropertyMap2 p_map2, 
                 bool verify = false) 
    : graph1_(graph1), graph2_(graph2), 
      p_map1_(p_map1), p_map2_(p_map2),
      verify_(verify) {}
  
  template <typename CorrespondenceMap1To2,
            typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
    
    if (verify_)
      std::cout << "Verify: " << std::boolalpha 
                << verify_vf2_sub_graph_iso(graph1_, graph2_, f)
                << std::endl;
    
    // Print sub graph isomorphism map
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) 
      std::cout << '(' << get(p_map1_,v) << ", " 
                << get(p_map2_, get(f, v)) << ") ";
    
    std::cout << std::endl;
    
    return true;
  }
  
private:
  const Graph1& graph1_;
  const Graph2& graph2_;
  
  const PropertyMap1& p_map1_;
  const PropertyMap2& p_map2_;
  
  const bool verify_;
};


int main(int argc, char** argv) {
  
  if (argc != 3) {
    cerr << "usage: " << argv[0] << " graph_small graph_large" << endl;
    return EXIT_FAILURE;
  }
  ifstream graph_small_file(argv[1]);
  ifstream graph_large_file(argv[2]);
  if (!graph_small_file || !graph_large_file) {
    cerr << "Files not found" << endl;
    return EXIT_FAILURE;
  }
  

  // Vertex properties
  typedef property <vertex_name_t, std::string> vertex_p;
  // adjacency_list-based type
#if 0
  typedef adjacency_list <vecS, vecS, bidirectionalS, vertex_p> graph_t;
#else
  typedef adjacency_list <vecS, vecS, undirectedS, vertex_p> graph_t;
#endif
  
  // Construct an empty graph_small and prepare the dynamic_property_maps.
  graph_t graph_small(0);
  dynamic_properties dp_small;
  
  property_map<graph_t, vertex_name_t>::type name_small =
    get(vertex_name, graph_small);
  dp_small.property("node_id", name_small);
  
  // Read graph_small
  bool status = read_graphviz(graph_small_file, graph_small, dp_small, "node_id");
  (void)status;
  
  // Construct an empty graph_large and prepare the dynamic_property_maps,
  // following the read_graphviz example
  graph_t graph_large(0);
  dynamic_properties dp_large;
  
  property_map<graph_t, vertex_name_t>::type name_large =
    get(vertex_name, graph_large);
  dp_large.property("node_id", name_large);
  
  // Read graph_large
  status = read_graphviz(graph_large_file, graph_large, dp_large, "node_id");
  
  
  // Create the call_back function
  typedef property_map<graph_t, vertex_name_t>::type p_map_t;
  print_callback<graph_t, graph_t, p_map_t, p_map_t> callback(graph_small, graph_large,
                                                              name_small, name_large, true);
  
  // Compute the sub-graph isomorphism mappings
#if 1
  bool ret = vf2_sub_graph_iso(graph_small, graph_large, callback);
  //bool ret = vf2_graph_iso(graph_small, graph_large, callback);
#else
  typedef graph_traits<graph_t>::vertex_descriptor vertex_t;
  typedef property_map<graph_t, vertex_index_t>::type index_map_t;
  
  index_map_t index_small = get(vertex_index, graph_small);
  index_map_t index_large = get(vertex_index, graph_large);
  
  graph_traits<graph_t>::vertex_iterator vi, vi_end;
  
  vector<vertex_t> vertex_order1;
  for (tie(vi, vi_end) = vertices(graph_small); vi != vi_end; ++vi)
    vertex_order1.push_back(*vi);
  
  bool ret = vf2_sub_graph_iso(graph_small, graph_large, callback, vertex_order1,
                               vertex_index1_map(index_small).vertex_index2_map(index_large));
  
#endif
  (void)ret;
  
  return 0;
}
