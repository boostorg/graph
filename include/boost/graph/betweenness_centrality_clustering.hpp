// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine
#ifndef BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP
#define BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP

#include <boost/graph/brandes_betweenness_centrality.hpp>
#include <boost/graph/graph_traits.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <algorithm>
#include <vector>
#include <boost/property_map.hpp>

namespace boost {

template<typename T>
struct bc_clustering_threshold
{
  explicit bc_clustering_threshold(T threshold) 
    : threshold(threshold), dividend(1.0) {}
  
  template<typename Graph>
  bc_clustering_threshold(T threshold, const Graph& g, bool normalize = true)
    : threshold(threshold), dividend(1.0)
  {
    if (normalize) {
      typename graph_traits<Graph>::vertices_size_type n = num_vertices(g);
      dividend = T((n - 1) * (n - 2)) / T(2);
    }
  }

  template<typename Graph, typename Edge>
  bool operator()(T max_centrality, Edge, const Graph&)
  {
    return (max_centrality / dividend) < threshold;
  }

 protected:
  T threshold;
  T dividend;
};

template<typename MutableGraph, typename Done, typename EdgeCentralityMap>
void 
betweenness_centrality_clustering(MutableGraph& g, Done done,
                                  EdgeCentralityMap edge_centrality)
{
  typedef typename property_traits<EdgeCentralityMap>::value_type
    centrality_type;
  typedef typename graph_traits<MutableGraph>::edge_iterator edge_iterator;
  typedef typename graph_traits<MutableGraph>::edge_descriptor edge_descriptor;
  typedef typename graph_traits<MutableGraph>::vertices_size_type
    vertices_size_type;

  if (edges(g).first == edges(g).second) return;

  // Function object that compares the centrality of edges
  indirect_cmp<EdgeCentralityMap, std::less<centrality_type> > 
    cmp(edge_centrality);

  bool is_done;
  do {
    brandes_betweenness_centrality(g, edge_centrality_map(edge_centrality));
    edge_descriptor e = *max_element(edges(g).first, edges(g).second, cmp);
    centrality_type max_centrality = get(edge_centrality, e);
    is_done = done(get(edge_centrality, e), e, g);
    if (!is_done) remove_edge(e, g);
  } while (!is_done && edges(g).first != edges(g).second);
}

} // end namespace boost

#endif // BOOST_GRAPH_BETWEENNESS_CENTRALITY_CLUSTERING_HPP
