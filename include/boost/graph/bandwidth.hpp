#ifndef BOOST_GRAPH_BANDWIDTH_HPP
#define BOOST_GRAPH_BANDWIDTH_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/detail/numeric_traits.hpp>

namespace boost {

  template <typename Graph, typename VertexIndexMap>
  typename graph_traits<Graph>::vertices_size_type
  ith_bandwidth(typename graph_traits<Graph>::vertex_descriptor i,
		const Graph& g,
		VertexIndexMap index)
  {
    typedef typename graph_traits<Graph>::vertices_size_type size_type;
    typedef typename detail::numeric_traits<size_type>::difference_type d_type;
    d_type b = 0;
    typename graph_traits<Graph>::out_edge_iterator e, end;
    for (tie(e, end) = out_edges(i, g); e != end; ++e) {
      b = std::max(b, std::abs(d_type(get(index, i)) - 
			       d_type(get(index, target(*e, g)))));
    }
    return b;
  }

  template <typename Graph>
  typename graph_traits<Graph>::vertices_size_type
  ith_bandwidth(typename graph_traits<Graph>::vertex_descriptor i,
		const Graph& g)
  {
    return ith_bandwidth(i, g, get(vertex_index, g));
  }

  template <typename Graph, typename VertexIndexMap>
  typename graph_traits<Graph>::vertices_size_type
  bandwidth(const Graph& g, VertexIndexMap index)
  {
    typename graph_traits<Graph>::vertices_size_type b = 0;
    typename graph_traits<Graph>::vertex_iterator i, end;
    for (tie(i, end) = vertices(g); i != end; ++i)
      b = std::max(b, ith_bandwidth(*i, g, index));
    return b;
  }

  template <typename Graph>
  typename graph_traits<Graph>::vertices_size_type
  bandwidth(const Graph& g)
  {
    return bandwidth(g, get(vertex_index, g));
  }

} // namespace boost

#endif // BOOST_GRAPH_BANDWIDTH_HPP
