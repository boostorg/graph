//  (C) Copyright David Abrahams 2000. Permission to copy, use,
//  modify, sell and distribute this software is granted provided this
//  copyright notice appears in all copies. This software is provided
//  "as is" without express or implied warranty, and with no claim as
//  to its suitability for any purpose.

#ifndef REVERSE_GRAPH_DWA092300_H_
# define REVERSE_GRAPH_DWA092300_H_

#include <boost/graph/detail/adjacency_list.hpp> // for adjacency_iterator or bidir_adj_iter

namespace boost {

template <class BidirectionalGraph>
class reverse_graph {
    typedef reverse_graph<BidirectionalGraph> Self;
 public:
    // Constructor
    reverse_graph(BidirectionalGraph& g) : m_g(g) {}
        
    // Graph requirements
    typedef typename graph_traits<BidirectionalGraph>::vertex_descriptor vertex_descriptor;
    typedef typename graph_traits<BidirectionalGraph>::edge_descriptor edge_descriptor;
    typedef typename graph_traits<BidirectionalGraph>::directed_category directed_category;
    typedef typename graph_traits<BidirectionalGraph>::edge_parallel_category edge_parallel_category;

    // IncidenceGraph requirements
    typedef typename graph_traits<BidirectionalGraph>::in_edge_iterator out_edge_iterator;
    typedef typename graph_traits<BidirectionalGraph>::degree_size_type degree_size_type;

    // BidirectionalGraph requirements
    typedef typename graph_traits<BidirectionalGraph>::out_edge_iterator in_edge_iterator;

    // AdjacencyGraph requirements
#if !defined BOOST_NO_ITERATOR_ADAPTORS
        typedef typename detail::adjacency_iterator<reverse_graph, vertex_descriptor,
          out_edge_iterator, out_edge_iterator>::type adjacency_iterator;
#else
        typedef detail::bidir_adj_iter<vertex_descriptor, out_edge_iterator,
                reverse_graph> adjacency_iterator;
#endif

    // VertexListGraph requirements
    typedef typename graph_traits<BidirectionalGraph>::vertex_iterator        vertex_iterator;

    // Stuff that shouldn't be required according to the docs 
    // except that we don't have partial specialization support on all
    // compilers

    typedef void edge_iterator;
    typedef typename graph_traits<BidirectionalGraph>::vertices_size_type     vertices_size_type;
    typedef void edges_size_type;
    
    // More typedefs used by detail::edge_property_map, vertex_property_map
    typedef typename BidirectionalGraph::edge_property_type edge_property_type;
    typedef typename BidirectionalGraph::vertex_property_type vertex_property_type;
    typedef typename BidirectionalGraph::graph_tag graph_tag;
    

    // would be private, but template friends aren't portable enough.
 // private:
    BidirectionalGraph& m_g;
};

template <class BidirectionalGraph>
inline reverse_graph<BidirectionalGraph>
make_reverse_graph(BidirectionalGraph& g)
{
    return reverse_graph<BidirectionalGraph>(g);
}

template <class BidirectionalGraph, class Property>
typename property_map<BidirectionalGraph, Property>::type
get(Property p, reverse_graph<BidirectionalGraph>& g)
{
  return get(p, g.m_g);
}

template <class BidirectionalGraph, class Property>
typename property_map<BidirectionalGraph, Property>::const_type
get(Property p, const reverse_graph<BidirectionalGraph>& g)
{
  return get(p, g.m_g);
}

template <class BidirectionalGraph, class Property, class Key>
typename property_traits<
  typename property_map<BidirectionalGraph, Property>::const_type
>::value_type
get(Property p, const reverse_graph<BidirectionalGraph>& g, const Key& k)
{
  return get(p, g.m_g, k);
}

template <class BidirectionalGraph, class Property, class Key, class Value>
void
put(Property p, const reverse_graph<BidirectionalGraph>& g, const Key& k,
    const Value& val)
{
  put(p, g.m_g, k, val);
}

template <class BidirectionalGraph>
std::pair<typename BidirectionalGraph::vertex_iterator,
          typename BidirectionalGraph::vertex_iterator>
vertices(const reverse_graph<BidirectionalGraph>& g)
{
    return vertices(g.m_g);
}

template <class BidirectionalGraph>
inline std::pair<typename BidirectionalGraph::in_edge_iterator, 
                 typename BidirectionalGraph::in_edge_iterator>
out_edges(const typename BidirectionalGraph::vertex_descriptor u,
          const reverse_graph<BidirectionalGraph>& g)
{
    return in_edges(u, g.m_g);
}

template <class BidirectionalGraph>
inline typename BidirectionalGraph::vertices_size_type
num_vertices(const reverse_graph<BidirectionalGraph>& g)
{
    return num_vertices(g.m_g);
}

template <class BidirectionalGraph>
inline typename BidirectionalGraph::degree_size_type
out_degree(const typename BidirectionalGraph::vertex_descriptor u,
           const reverse_graph<BidirectionalGraph>& g)
{
    return in_degree(u, g.m_g);
}

template <class BidirectionalGraph>
inline std::pair<typename BidirectionalGraph::edge_descriptor, bool>
edge(const typename BidirectionalGraph::vertex_descriptor u,
     const typename BidirectionalGraph::vertex_descriptor v,
     const reverse_graph<BidirectionalGraph>& g)
{
    return edge(v, u, g);
}

template <class BidirectionalGraph>
inline std::pair<typename BidirectionalGraph::out_edge_iterator, 
    typename BidirectionalGraph::out_edge_iterator>
in_edges(const typename BidirectionalGraph::vertex_descriptor u,
         const reverse_graph<BidirectionalGraph>& g)
{
    return out_edges(u, g.m_g);
}

template <class BidirectionalGraph>
inline std::pair<typename reverse_graph<BidirectionalGraph>::adjacency_iterator, 
    typename reverse_graph<BidirectionalGraph>::adjacency_iterator>
adjacent_vertices(const typename BidirectionalGraph::vertex_descriptor u,
                  const reverse_graph<BidirectionalGraph>& g)
{
    typedef reverse_graph<BidirectionalGraph> Graph;
    typename Graph::out_edge_iterator first, last;
    tie(first, last) = out_edges(u, g);
    typedef typename Graph::adjacency_iterator adjacency_iterator;
    return std::make_pair(adjacency_iterator(first, const_cast<Graph*>(&g)),
                          adjacency_iterator(last, const_cast<Graph*>(&g)));
}

template <class BidirectionalGraph>
inline typename BidirectionalGraph::degree_size_type
in_degree(const typename BidirectionalGraph::vertex_descriptor u,
          const reverse_graph<BidirectionalGraph>& g)
{
    return out_degree(u, g.m_g);
}

template <class Edge, class BidirectionalGraph>
inline typename graph_traits<BidirectionalGraph>::vertex_descriptor
source(const Edge& e, const reverse_graph<BidirectionalGraph>& g)
{
    return target(e, g.m_g);
}

template <class Edge, class BidirectionalGraph>
inline typename graph_traits<BidirectionalGraph>::vertex_descriptor
target(const Edge& e, const reverse_graph<BidirectionalGraph>& g)
{
    return source(e, g.m_g);
}

} // namespace boost

#endif
