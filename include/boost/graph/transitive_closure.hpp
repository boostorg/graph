// Copyright (C) 2001 Vladimir Prus <ghost@cs.msu.su>
// Copyright (C) 2001 Jeremy Siek <jsiek@cs.indiana.edu>
// Permission to copy, use, modify, sell and distribute this software is
// granted, provided this copyright notice appears in all copies and 
// modified version are clearly marked as such. This software is provided
// "as is" without express or implied warranty, and with no claim as to its
// suitability for any purpose.

#ifndef BOOST_TRANSITIVE_CLOSURE_HPP
#define BOOST_TRANSITIVE_CLOSURE_HPP

#include <vector>
#include <functional>
#include <boost/compose.hpp>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

// For a description of the implementation see ../doc/transitive_closure.pdf.

namespace boost {

  namespace detail {

    void union_successor_sets(const std::vector<std::size_t>& s1,
  			  const std::vector<std::size_t>& s2,
  			  std::vector<std::size_t>& s3)
    {
      for (std::size_t k = 0; k < s1.size(); ++k)
        s3[k] = std::min(s1[k], s2[k]);
    }

    // These classes really don't belong here!
    // They should be moved somewhere
    template<class Container, class ST = std::size_t,
         class RT = typename Container::value_type>
    struct const_subscript_t : std::unary_function<ST, RT>
    {
      const_subscript_t(const Container& c) : container(&c) {};
      typename Container::const_reference
      operator()(const ST& i) const 
      { return (*container)[i]; }
    protected:
      const Container* container;
    };
    template <typename Container, typename ST = std::size_t, 
      typename VT = typename Container::value_type>
    struct subscript_t : std::unary_function<ST, VT> {
      subscript_t(Container& c) : container(&c) {};
      typename Container::reference
      operator()(const ST& i) const { return (*container)[i]; }
    protected:
      Container *container;
    };

    template<class Container>
    const_subscript_t<Container>
    subscript(const Container& c)
    { return const_subscript_t<Container>(c); }

    template <typename Container>
    subscript_t<Container> subscript(Container& c)
    { return subscript_t<Container>(c); }

  } // namespace detail
  
  template <typename Graph, typename GraphTC, typename VertexIndexMap>
  void transitive_closure(const Graph& g, GraphTC& tc, 
			  VertexIndexMap index_map)
  {
    typedef typename graph_traits<Graph>::vertex_descriptor vertex;
    typedef typename graph_traits<Graph>::edge_descriptor edge;
    typedef typename graph_traits<Graph>::vertex_iterator vertex_iterator;
    typedef typename property_traits<VertexIndexMap>::value_type size_type;
    typedef typename graph_traits<Graph>::adjacency_iterator 
      adjacency_iterator;

    function_requires< VertexListGraphConcept<Graph> >();
    function_requires< AdjacencyGraphConcept<Graph> >();
    function_requires< VertexMutableGraphConcept<GraphTC> >();
    function_requires< EdgeMutableGraphConcept<GraphTC> >();

    // Compute strongly connected components of the graph
    typedef size_type cg_vertex;
    std::vector<cg_vertex> component_number_vec(num_vertices(g));
    iterator_property_map<cg_vertex*, VertexIndexMap> 
      component_number(&component_number_vec[0], index_map);

    int num_scc = strong_components(g, component_number,
      vertex_index_map(index_map));
    std::vector< std::vector<vertex> > components;
    components.resize(num_scc);
    vertex_iterator vi, vi_end;
    for (tie(vi, vi_end) = vertices(g); vi != vi_end; ++vi)
      components[component_number[*vi]].push_back(*vi);

    // Construct the condensation graph   
    typedef std::vector< std::vector<cg_vertex> > CG_t;
    CG_t CG(num_scc);
    for (cg_vertex s = 0; s < components.size(); ++s) {
      std::vector<cg_vertex> adj;
      for (size_type i = 0; i < components[s].size(); ++i) {
        vertex u = components[s][i];
        adjacency_iterator v, v_end;
        for (tie(v, v_end) = adjacent_vertices(u, g); v != v_end; ++v) {
          cg_vertex t = component_number[*v];
          if (s != t) // Avoid loops in the condensation graph
            adj.push_back(t);
        }
      }
      std::sort(adj.begin(), adj.end());
      std::vector<cg_vertex>::iterator
	di = std::unique(adj.begin(), adj.end());
      if (di != adj.end())
        adj.erase(di, adj.end());        
      CG[s] = adj;
    }

    // topological sort the condensation graph
    std::vector<cg_vertex> topo_order;
    std::vector<cg_vertex> topo_number(num_vertices(CG));
    topological_sort(CG, std::back_inserter(topo_order), 
      vertex_index_map(identity_property_map()));
    std::reverse(topo_order.begin(), topo_order.end());
    size_type n = 0;
    for (std::vector<cg_vertex>::iterator i = topo_order.begin();
         i != topo_order.end(); ++i)
      topo_number[*i] = n++;
    
    for (size_type i = 0; i < num_vertices(CG); ++i)
      std::sort(CG[i].begin(), CG[i].end(), 
		compose_f_gx_hy(std::less<cg_vertex>(),
				detail::subscript(topo_number),
				detail::subscript(topo_number)));   

    // Decompose the condensation graph into chains
    std::vector< std::vector<cg_vertex> > chains;
    {
      std::vector<cg_vertex> in_a_chain(num_vertices(CG));
      for (std::vector<cg_vertex>::iterator i = topo_order.begin();
	   i != topo_order.end(); ++i) {
	cg_vertex v = *i;
	if (!in_a_chain[v]) {
	  chains.resize(chains.size() + 1);
	  std::vector<cg_vertex>& chain = chains.back();
	  for (;;) {
	    chain.push_back(v);
	    in_a_chain[v] = true;
	    graph_traits<CG_t>::adjacency_iterator adj_first, adj_last;
	    tie(adj_first, adj_last) = adjacent_vertices(v, CG);
	    graph_traits<CG_t>::adjacency_iterator next
	      = std::find_if(adj_first, adj_last,
			     not1(detail::subscript(in_a_chain)));
	    if (next != adj_last)
	      v = *next;
	    else
	      break; // end of chain, dead-end
	    
	  }
	}
      }
    }
    std::vector<size_type> chain_number(num_vertices(CG));
    std::vector<size_type> pos_in_chain(num_vertices(CG));
    for (size_type i = 0; i < chains.size(); ++i)
      for (size_type j = 0; j < chains[i].size(); ++j) {
	cg_vertex v = chains[i][j];
	chain_number[v] = i;
	pos_in_chain[v] = j;
      }             
    
    // Compute successors sets of every vertex in the condensation graph
    cg_vertex inf = std::numeric_limits<cg_vertex>::max();
    std::vector< std::vector<cg_vertex> > 
      successors(num_vertices(CG),
		 std::vector<cg_vertex>(chains.size(), inf));
    for (std::vector<cg_vertex>::reverse_iterator i = topo_order.rbegin();
	 i != topo_order.rend(); ++i) {
      cg_vertex u = *i;
      graph_traits<CG_t>::adjacency_iterator adj, adj_last;
      for (tie(adj, adj_last) = adjacent_vertices(u, CG);
	   adj != adj_last; ++adj) {
	cg_vertex v = *adj;
	if (topo_number[v] < successors[v][chain_number[v]]) {
	  // Succ(u) = Succ(u) U Succ(v)
	  detail::union_successor_sets(successors[u], successors[v], 
				       successors[u]);
	  // Succ(u) = Succ(u) U {v}
	  successors[u][chain_number[v]] = topo_number[v];
	}
      }
    }

    // Build the transitive closure of the condensation graph
    // based on the successor sets.
    for (size_type i = 0; i < CG.size(); ++i)
      CG[i].clear();
    for (size_type i = 0; i < CG.size(); ++i) 
      for (size_type j = 0; j < chains.size(); ++j) {
	size_type topo_num = successors[i][j];
	if (topo_num < inf) {
	  cg_vertex v = topo_order[topo_num];
	  for (size_type k = pos_in_chain[v]; k < chains[j].size(); ++k)
	    CG[i].push_back(chains[j][k]);
	}
      }
    
    // Build transitive closure of the original graph

    // Add vertices to the transitive closure graph
    typedef typename graph_traits<GraphTC>::vertex_descriptor tc_vertex;
    std::vector<tc_vertex> to_tc_vec(num_vertices(g));
    iterator_property_map<tc_vertex*, VertexIndexMap> 
      to_tc(&to_tc_vec[0], index_map);
    {
      vertex_iterator i, i_end;
      for (tie(i, i_end) = vertices(g); i != i_end; ++i)
	to_tc[*i] = add_vertex(tc);
    }

    // Add edges between all the vertices in two adjacent SCCs
    graph_traits<CG_t>::vertex_iterator si, si_end;
    for (tie(si, si_end) = vertices(CG); si != si_end; ++si) {
      cg_vertex s = *si;
      graph_traits<CG_t>::adjacency_iterator i, i_end;
      for (tie(i, i_end) = adjacent_vertices(s, CG); i != i_end; ++i) {
        cg_vertex t = *i;
        for (size_type k = 0; k < components[s].size(); ++k)
          for (size_type l = 0; l < components[t].size(); ++l)
            add_edge(to_tc[components[s][k]], to_tc[components[t][l]], tc);
      }
    }
    // Add edges connecting all vertices in a SCC
    for (size_type i = 0; i < components.size(); ++i)
      if (components[i].size() > 1)
        for (size_type k = 0; k < components[i].size(); ++k)
          for (size_type l = 0; l < components[i].size(); ++l) {
            vertex u = components[i][k], v = components[i][l];
            if (u != v)
              add_edge(to_tc[u], to_tc[v], tc);
          }    
  }
  
  template <typename Graph, typename GraphTC>
  void transitive_closure(const Graph& g, GraphTC& tc)
  {
    transitive_closure(g, tc, get(vertex_index, g));
  }

  template<class G>
  void warshall_transitive_closure(G& g)
  {
    using namespace boost;
    typedef typename graph_traits<G>::vertex_descriptor vertex;
    typedef typename graph_traits<G>::vertex_iterator vertex_iterator;

    function_requires< AdjacencyMatrixConcept<G> >();
    function_requires< EdgeMutableGraphConcept<G> >();

    // Matrix form:
    // for k
    //  for i
    //    if A[i,k]
    //      for j
    //        A[i,j] = A[i,j] | A[k,j]
    vertex_iterator ki, ke, ii, ie, ji, je;
    for (boost::tie(ki, ke) = vertices(g); ki != ke; ++ki)
      for (boost::tie(ii, ie) = vertices(g); ii != ie; ++ii) 
        if (edge(*ii, *ki, g).second)
          for (boost::tie(ji, je) = vertices(g); ji != je; ++ji)
            if (!edge(*ii, *ji, g).second &&
              edge(*ki, *ji, g).second)
            {
              add_edge(*ii, *ji, g);
            }               
  }

  template<class G>
  void warren_transitive_closure(G& g)
  {
    using namespace boost;
    typedef typename graph_traits<G>::vertex_descriptor vertex;
    typedef typename graph_traits<G>::vertex_iterator vertex_iterator;

    function_requires< AdjacencyMatrixConcept<G> >();
    function_requires< EdgeMutableGraphConcept<G> >();

    // Make sure second loop will work  
    if (num_vertices(g) == 0)
      return;

    // for i = 2 to n
    //    for k = 1 to i - 1 
    //      if A[i,k]
    //        for j = 1 to n
    //          A[i,j] = A[i,j] | A[k,j]

    vertex_iterator ic, ie, jc, je, kc, ke;
    for (boost::tie(ic, ie) = vertices(g), ++ic; ic != ie; ++ic)
      for (boost::tie(kc, ke) = vertices(g); *kc != *ic; ++kc)
        if (edge(*ic, *kc, g).second)
          for (boost::tie(jc, je) = vertices(g); jc != je; ++jc)
            if (!edge(*ic, *jc, g).second &&
              edge(*kc, *jc, g).second)
            {
              add_edge(*ic, *jc, g);
            }

    //  for i = 1 to n - 1
    //    for k = i + 1 to n
    //      if A[i,k]
    //        for j = 1 to n
    //          A[i,j] = A[i,j] | A[k,j]

    for (boost::tie(ic, ie) = vertices(g), --ie; ic != ie; ++ic)
      for (kc = ic, ke = ie, ++kc; kc != ke; ++kc)
        if (edge(*ic, *kc, g).second)
          for (boost::tie(jc, je) = vertices(g); jc != je; ++jc)
            if (!edge(*ic, *jc, g).second &&
              edge(*kc, *jc, g).second)
            {
              add_edge(*ic, *jc, g);
            }                     
  }
    
} // namespace boost

#endif // BOOST_TRANSITIVE_CLOSURE_HPP
