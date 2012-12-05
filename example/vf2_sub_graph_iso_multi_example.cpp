//=======================================================================
// Copyright (C) 2012 Flavio De Lorenzi (fdlorenzi@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#include <string>
#include <vector>
#include <utility>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/vf2_sub_graph_iso.hpp>
using namespace boost;


template <typename Graph1,
          typename Graph2,
          typename EdgeCompatibilityPredicate,
          typename VertexCompatibilityPredicate>
struct my_print_callback {
  
  my_print_callback(const Graph1& graph1, const Graph2& graph2, 
                    EdgeCompatibilityPredicate edge_comp, 
                    VertexCompatibilityPredicate vertex_comp) 
    : graph1_(graph1), graph2_(graph2), edge_comp_(edge_comp), vertex_comp_(vertex_comp) {}
  
  template <typename CorrespondenceMap1To2,
            typename CorrespondenceMap2To1>
  bool operator()(CorrespondenceMap1To2 f, CorrespondenceMap2To1) const {
    
    std::cout << "Verify: " << std::boolalpha 
              << verify_vf2_sub_graph_iso(graph1_, graph2_, f, edge_comp_, vertex_comp_)
              << std::endl;
    
    // Print sub graph isomorphism map
    BGL_FORALL_VERTICES_T(v, graph1_, Graph1) 
      std::cout << '(' << v << ", " << get(f, v) << ") ";
    
    std::cout << std::endl;
    
    return true;
  }
  
private:
  const Graph1& graph1_;
  const Graph2& graph2_;
  EdgeCompatibilityPredicate edge_comp_;
  VertexCompatibilityPredicate vertex_comp_;
};




int main() {
  typedef property<edge_name_t, char> edge_property;
  typedef property<vertex_name_t, char> vertex_property;
  
  //typedef adjacency_list<vecS, vecS, bidirectionalS, vertex_property, edge_property> graph_type;
  typedef adjacency_list<vecS, vecS, undirectedS, vertex_property, edge_property> graph_type;
  //typedef adjacency_list<setS, vecS, bidirectionalS, vertex_property, edge_property> graph_type;
  //typedef adjacency_list<setS, vecS, undirectedS, vertex_property, edge_property> graph_type;
  
  // Build graph1
  graph_type graph1;
  
  add_vertex(vertex_property('a'), graph1);
  add_vertex(vertex_property('a'), graph1);
  add_vertex(vertex_property('a'), graph1);
  
  add_edge(0, 1, edge_property('b'), graph1); 
  add_edge(0, 1, edge_property('b'), graph1); 
  add_edge(0, 1, edge_property('d'), graph1); 
  
  add_edge(1, 2, edge_property('s'), graph1); 
  
  add_edge(2, 2, edge_property('l'), graph1); 
  add_edge(2, 2, edge_property('l'), graph1); 
  

  // Build graph2
  graph_type graph2;
  
  add_vertex(vertex_property('a'), graph2);
  add_vertex(vertex_property('a'), graph2);
  add_vertex(vertex_property('a'), graph2);
  add_vertex(vertex_property('a'), graph2);
  add_vertex(vertex_property('a'), graph2);
  add_vertex(vertex_property('a'), graph2);
  
  add_edge(0, 1, edge_property('a'), graph2); 
  add_edge(0, 1, edge_property('a'), graph2); 
  add_edge(0, 1, edge_property('b'), graph2); 

  
  add_edge(1, 2, edge_property('s'), graph2); 
  
  add_edge(2, 3, edge_property('b'), graph2); 
  add_edge(2, 3, edge_property('d'), graph2); 
  add_edge(2, 3, edge_property('b'), graph2); 
  
  add_edge(3, 4, edge_property('s'), graph2); 
  
  add_edge(4, 4, edge_property('l'), graph2); 
  add_edge(4, 4, edge_property('l'), graph2); 

  add_edge(4, 5, edge_property('c'), graph2); 
  add_edge(4, 5, edge_property('c'), graph2); 
  add_edge(4, 5, edge_property('c'), graph2); 
  
  add_edge(5, 0, edge_property('s'), graph2); 
  
  // create predicates
  typedef property_map<graph_type, vertex_name_t>::type vertex_name_map_t;
  
  typedef property_map_compatible<vertex_name_map_t, vertex_name_map_t> vertex_comp_t;
  vertex_comp_t vertex_comp =
    make_property_map_compatible(get(vertex_name, graph1), get(vertex_name, graph2));
  
  typedef property_map<graph_type, edge_name_t>::type edge_name_map_t;
  
  typedef property_map_compatible<edge_name_map_t, edge_name_map_t> edge_comp_t;
  edge_comp_t edge_comp =
    make_property_map_compatible(get(edge_name, graph1), get(edge_name, graph2));
  
  
  graph_traits<graph_type>::vertex_iterator vi, vi_end;
  
  // define the order in whcih vertices of graph1 are examined
  std::vector<graph_traits<graph_type>::vertex_descriptor> vertex_order1;
  for (tie(vi, vi_end) = vertices(graph1); vi != vi_end; ++vi)
    vertex_order1.push_back(*vi);
  
  // true instructs callback to verify a map using
  // verify_vf2_sub_graph_iso
  my_print_callback<graph_type, graph_type, edge_comp_t, vertex_comp_t> 
    callback(graph1, graph2, edge_comp, vertex_comp);
  
  bool ret = vf2_sub_graph_iso(graph1, graph2, callback, vertex_order1,
                               edges_equivalent(edge_comp).vertices_equivalent(vertex_comp));
  
  (void)ret;
  
  return 0;
}
