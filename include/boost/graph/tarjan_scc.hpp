#ifndef BOOST_TARJAN_SCC_HPP
#define BOOST_TARJAN_SCC_HPP

#include <boost/graph/depth_first_search.hpp>

namespace boost {

  namespace detail {
  
    template <typename ComponentMap, typename RootMap, typename DFSNumberMap,
              typename Stack>
    class tarjan_scc_visitor : public dfs_visitor<> {
      typedef typename property_traits<ComponentMap>::value_type comp_type;
      typedef typename property_traits<DFSNumberMap>::value_type time_type;
    public:
      tarjan_scc_visitor(ComponentMap comp_map, RootMap r, DFSNumberMap n, 
			 comp_type& c_, Stack& s_)
	: c(c_), comp(comp_map), root(r), dfs_number(n),
	  dfs_time(time_type()), s(s_) { }

      template <typename Vertex, typename Graph>
      void discover_vertex(Vertex v, Graph& ) {
	root[v] = v;
	comp[v] = std::numeric_limits<comp_type>::max();
	dfs_number[v] = dfs_time++;
	s.push(v);
      }
      template <typename Vertex, typename Graph>
      void finish_vertex(Vertex v, Graph& g) {
	Vertex w;
	typename graph_traits<Graph>::out_edge_iterator ei, ei_end;
	for (tie(ei, ei_end) = out_edges(v, g); ei != ei_end; ++ei) {
	  w = target(*ei, g);
	  if (comp[w] == std::numeric_limits<comp_type>::max())
	    root[v] = this->min_dfs_number(root[v], root[w]);
	}
	if (root[v] == v) {
	  do {
	    w = s.top(); s.pop();
	    comp[w] = c;
	  } while (w != v);
	  ++c;
	}
      }
    private:
      template <typename Vertex>
      Vertex min_dfs_number(Vertex u, Vertex v) {
	return dfs_number[u] < dfs_number[v] ? u : v;
      }

      comp_type& c;
      ComponentMap comp;
      RootMap root;
      DFSNumberMap dfs_number;
      time_type dfs_time;
      Stack& s;
    };

  } // namespace detail 

  template <typename Graph, typename ComponentMap, typename RootMap,
            typename ColorMap, typename DFSNumberMap>
  typename property_traits<ComponentMap>::value_type
  tarjan_scc(Graph& g,          // Input
	     ComponentMap comp, // Output
	     // Internal record keeping
	     RootMap root, ColorMap color, DFSNumberMap dfs_number_map)
  {
    typename property_traits<ComponentMap>::value_type total = 0;
    typedef std::stack<typename graph_traits<Graph>::vertex_descriptor> Stack;
    Stack s;
    detail::tarjan_scc_visitor<ComponentMap, RootMap, DFSNumberMap, Stack> 
      vis(comp, root, dfs_number_map, total, s);
    depth_first_search(g, vis, color);
    return total;
  }
} // namespace boost

#endif // BOOST_TARJAN_SCC_HPP
