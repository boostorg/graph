#ifndef BOOST_GRAPH_COPY_HPP
#define BOOST_GRAPH_COPY_HPP

#include <boost/config.hpp>
#include <vector>
#include <boost/graph/graph_traits.hpp>
#include <boost/property_map.hpp>

// UNDER CONSTRUCTION

namespace boost {

  template <typename Graph1, typename Graph2>
  struct edge_copier {
    edge_copier(const Graph1& g1, Graph2& g2)
      : edge_all_map1(get(edge_all, g1)), edge_all_map2(get(edge_all, g2)) { }
    
    template <typename Edge1, typename Edge2>
    void operator()(const Edge1& e1, Edge2& e2) const {
      put(edge_all_map2, e2, get(edge_all_map1, e1));
    }
    typename property_map<Graph1, edge_all_t>::const_type edge_all_map1;
    mutable typename property_map<Graph2, edge_all_t>::type edge_all_map2;
  };
  template <typename Graph1, typename Graph2>
  inline edge_copier<Graph1,Graph2>
  make_edge_copier(const Graph1& g1, Graph2& g2)
  {
    return edge_copier<Graph1,Graph2>(g1, g2);
  }

  template <typename Graph1, typename Graph2>
  struct vertex_copier {
    vertex_copier(const Graph1& g1, Graph2& g2)
      : vertex_all_map1(get(vertex_all, g1)), 
      vertex_all_map2(get(vertex_all, g2)) { }
    
    template <typename Vertex1, typename Vertex2>
    void operator()(const Vertex1& v1, Vertex2& v2) const {
      put(vertex_all_map2, v2, get(vertex_all_map1, v1));
    }
    typename property_map<Graph1, vertex_all_t>::const_type vertex_all_map1;
    mutable typename property_map<Graph2, vertex_all_t>::type vertex_all_map2;
  };
  template <typename Graph1, typename Graph2>
  inline vertex_copier<Graph1,Graph2>
  make_vertex_copier(const Graph1& g1, Graph2& g2)
  {
    return vertex_copier<Graph1,Graph2>(g1, g2);
  }


  // Copy all the vertices and edges of graph g_in into graph g_out.
  // The copy_vertex and copy_edge function objects control how vertex
  // and edge properties are copied.
  template <typename Graph, typename MutableGraph, 
            typename CopyVertex, typename CopyEdge, 
            typename VertexIndexMap, typename Old2NewVertexMap>
  void copy_graph(Graph& g_in, MutableGraph& g_out, 
                  CopyVertex copy_vertex, CopyEdge copy_edge,
                  OrigVertexIndexMap v_index,
                  Copy2OrigVertexIndexMap vertex_map)
  {
    // assert Graph::directed_category == MutableGraph::directed_category
    // or allow conversions? 

    // dispatch based on directed_category and traversal_category...

    typename graph_traits<Graph>::vertex_iterator vi, vi_end;
    for (tie(vi, vi_end) = vertices(g_in); vi != vi_end; ++vi) {
      typename graph_traits<MutableGraph>::vertex_descriptor
        new_v = add_vertex(g_out);
      put(vertex_map, *vi, new_v);
      copy_vertex(*vi, new_v);
    }

    typename graph_traits<Graph>::edge_iterator ei, ei_end;
    for (tie(ei, ei_end) = edges(g_in); ei != ei_end; ++ei) {
      typename graph_traits<MutableGraph>::edge_descriptor new_e;
      bool inserted;
      tie(new_e, inserted) = add_edge(get(vertex_map, source(*ei, g_in)), 
                                      get(vertex_map, target(*ei, g_in)),
                                      g_out);
      copy_edge(*ei, new_e);
    }
  }
  template <typename Graph, typename MutableGraph>
  void copy_graph(Graph& g_in, MutableGraph& g_out)
  {
    std::vector<typename graph_traits<Graph>::vertex_descriptor> 
      vertex_map(num_vertices(g_in));
    copy_graph(g_in, g_out, 
               make_vertex_copier(g_in, g_out), 
               make_edge_copier(g_in, g_out), 
               get(vertex_index, g_in), 
               make_iterator_property_map(vertex_map.begin(), 
                                          get(vertex_index, g_in)));
  }

  template <typename Graph, typename MutableGraph, typename Old2NewVertexMap>
  void copy_graph(Graph& g_in, MutableGraph& g_out, 
                  Old2NewVertexMap& vertex_map)
  {
    copy_graph(g_in, g_out, default_copier(), default_copier(), 
               get(vertex_index, g), vertex_map);
  }
  
  // Copy all the vertices and edges of graph g_in that are reachable
  // from the source vertex into graph g_out. Return the vertex
  // in g_out that matches the source vertex of g_in.
  template <typename Graph, typename MutableGraph, 
           typename CopyVertex, typename CopyEdge>
  typename graph_traits<Graph>::vertex_descriptor
  copy_graph(typename graph_traits<Graph>::vertex_descriptor src,
             Graph& g_in, 
             MutableGraph& g_out, 
             CopyVertex copy_vertex, CopyEdge copy_edge)
  {
    // dispatch based on directed_category and traversal_category...
    
  }
  template <typename Graph, typename MutableGraph, 
           typename CopyVertex>
  typename graph_traits<Graph>::vertex_descriptor
  copy_graph(typename graph_traits<Graph>::vertex_descriptor src,
             Graph& g_in, 
             MutableGraph& g_out, 
             CopyVertex copy_vertex)
  {
    return copy_graph(src, g_in, g_out, copy_vertex, default_copier());
  }
  template <typename Graph, typename MutableGraph>
  typename graph_traits<Graph>::vertex_descriptor
  copy_graph(typename graph_traits<Graph>::vertex_descriptor src,
             Graph& g_in, 
             MutableGraph& g_out)
  {
    return copy_graph(src, g_in, g_out, default_copier(), default_copier());
  }
  
} // namespace boost

#endif // BOOST_GRAPH_COPY_HPP
