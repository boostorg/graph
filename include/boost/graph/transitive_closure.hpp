// Copyright (C) 2001 Vladimir Prus <ghost@cs.msu.su>
// Permission to copy, use, modify, sell and distribute this software is
// granted, provided this copyright notice appears in all copies and 
// modified version are clearly marked as such. This software is provided
// "as is" without express or implied warranty, and with no claim as to its
// suitability for any purpose.

#include <vector>
#include <functional>
#include <boost/compose.hpp>
#include <boost/graph/vector_as_graph.hpp>
#include <boost/graph/strong_components.hpp>
#include <boost/graph/topological_sort.hpp>

namespace boost {

  namespace detail {
    // These classes really don't belong here!
    // They should be moved somewhere
    template<class Container, class ST = std::size_t,
         class RT = typename Container::value_type>
    struct const_subscript_t : std::unary_function<ST, RT>
    {
      const_subscript_t(const Container& c) : container(&c) {};
      const RT& operator()(const ST& i) const 
      { return (*container)[i]; }

    protected:
      const Container* container;
    };

    template<class Container, class ST = std::size_t, 
      class RT = typename Container::value_type>
    struct subscript_t : std::unary_function<ST, RT>
    {
      subscript_t(Container& c) : container(&c) {};
      RT& operator()(const ST& i) const
      {   return (*container)[i]; }

    protected:
      Container *container;
    };

    struct subscript_vb_t : std::unary_function<std::size_t, bool>
    {
      subscript_vb_t(std::vector<bool>& c) : container(&c) {};
      std::vector<bool>::reference operator()(std::size_t i) const
      {   return (*container)[i]; }
      
    protected:
      std::vector<bool> *container;
    }; 

    template<class Container>
    const_subscript_t<Container>
    subscript(const Container& c)
    { return const_subscript_t<Container>(c); }

    
    template<class Container>
    subscript_t<Container>
    subscript(Container &c)
    { return subscript_t<Container>(c); }

    inline
    subscript_vb_t
    subscript(std::vector<bool>& v)
    {
      return subscript_vb_t(v);
    }   

    template<class T>
    struct truth : std::unary_function<bool, T>
    {
      bool operator()(const T&) { return true; }
    };

  }

  namespace detail {
    // The following functions are used in implementation of the algorithm
    // They are not generic in any way, since it would require much work
    // for no benefit.

    /** Makes adjacencent vertices of any given vertex to appear in
      topological order.
    */
    template <class Vertex>
    void topologically_sort_edges(std::vector< std::vector<Vertex> >& g, 
                    const std::vector<Vertex>& top_num)
    {
      for (size_t i = 0; i < g.size(); ++i) {
        sort(g[i].begin(), g[i].end(), 
           compose_f_gx_hy(std::less<Vertex>(),
                   subscript(top_num),
                   subscript(top_num)));    
      }
    }

    /** Computed decomposition of vertices into disjoint chains,
      where chains are pathes in graph such that vertices occur
      in them in topological order. The averange number of chains
      for this algorithm in G(n, p) graphs is O(ln(np)/p).      
    */
    template <class Vertex>
    void compute_chains(const std::vector< std::vector<Vertex> >& g,
              const std::vector<Vertex>& top_order,
              std::vector< std::vector<Vertex> >& chains)
    {
      std::vector<bool> in_chains(g.size());

      for (size_t i = 0; i < top_order.size(); ++i) {

        Vertex v = top_order[i];
        if (!in_chains[v]) {

          chains.resize(chains.size()+1);
          std::vector<Vertex>& chain = chains.back();

          for(;;) {
            
            chain.push_back(v);
            in_chains[v] = true;

            std::vector<Vertex>::const_iterator next;
            next = find_if(g[v].begin(), g[v].end(),
                     not1(subscript(in_chains)));
            if (next != g[v].end())
              v = *next;
            else
              break;
          }
        }
      }
    }

    template <class Vertex>
    void acyclic_closure(std::vector< std::vector<Vertex> >& g)
    {
      // The simplest algorithm would be
      //  for u \in reverse_topological_order
      //    for (v in adj(u))
      //      if (v \notin succ(u))
      //        succ(u) = succ(u) | { v } \cup succ(v)
      //  where succ(v) is the set of all transitive succesors of v
      //  if succ union takes linear time, this is clearly
      //  0(n*e) algorithm.
      //  Different representation for succ impvored averange
      //  performance, but not worst case behaviour.
      //  In this case, chain decomposition is used -- division
      //  of V into disjoint pathes C, where vertices in each
      //  path are topologically ordered. So, succ(u) will
      //  be a vector, telling for each chain index of element
      //  of chain with the smallest topological number.

      std::vector<Vertex> top_order;
      std::vector<Vertex> top_num(g.size());
      typedef typename std::vector<Vertex>::size_type size_type;
      {
        identity_property_map id;
        topological_sort(g, back_inserter(top_order), 
			 vertex_index_map(id));
        std::reverse(top_order.begin(), top_order.end());

        for (size_type i = 0; i < top_order.size(); ++i)
          top_num[top_order[i]] = i;
      }

      topologically_sort_edges(g, top_num);

      std::vector< std::vector<Vertex> > chains;
      std::vector<Vertex> chain_no(g.size()), index_in_chain(g.size());
      {
        compute_chains(g, top_order, chains);

        for (size_type i = 0; i < chains.size(); ++i)
          for (size_type j = 0; j < chains[i].size(); ++j) {
            Vertex v = chains[i][j];
            chain_no[v] = i;
            index_in_chain[v] = j;
          }                         
      }
      
#ifdef PRINTS
      cout << "Chains found are :\n" << multiline << chains << endl;
#endif

      Vertex inf = std::numeric_limits<Vertex>::max();
      std::vector< std::vector<Vertex> > successors;
      {
        successors.resize(g.size(), std::vector<Vertex>(chains.size(), inf));

	for (std::vector<Vertex>::reverse_iterator i = top_order.rbegin();
	     i != top_order.rend(); ++i) {
          Vertex v = *i;

          for (size_type j = 0; j < g[v].size(); ++j) {
            Vertex av = g[v][j];

            if (top_num[av] < successors[v][chain_no[av]]) {
              
              for (size_type k = 0; k < chains.size(); ++k)
                successors[v][k] = std::min(successors[v][k],
                             successors[av][k]);
              successors[v][chain_no[av]] = top_num[av];
            }
          }
        }
      }

#ifdef PRINTS
      cout << "Successors sets\n" << multiline << successors << endl;
#endif


      for (size_type i = 0; i < g.size(); ++i)
        g[i].clear();

      for (size_type i = 0; i < g.size(); ++i) 
        for (size_type j = 0; j < chains.size(); ++j) {
          Vertex s = successors[i][j];

          if (s < inf) {
            Vertex v = top_order[s];
            for (size_type k = index_in_chain[v]; k < chains[j].size(); ++k)
              g[i].push_back(chains[j][k]);
          }
        }
    }
  }


  /** Computes transitive closure of graph g. The algorithm used has
    worst-case complexity 0(ne) and averange complexity on G(n, p) 
    graphs is O(n ln(np)/p). Note: algorithm temporary removed all
    the edges, so any information assosiated with edges is lost.
  */
  template<class G>
  void transitive_closure(G& g)
  {
    using namespace detail;

    function_requires< AdjacencyGraphConcept<G> >();
    function_requires< MutableGraphConcept<G> >();
     
    typedef typename graph_traits<G>::vertex_descriptor vertex;
    typedef typename graph_traits<G>::edge_descriptor edge;
    typedef typename graph_traits<G>::vertex_iterator vertex_iterator;
    typedef typename graph_traits<G>::adjacency_iterator adjacency_iterator;

    //  Find SCCs
    std::vector<vertex> component_no(num_vertices(g));
    std::vector< std::vector<vertex> > components;
    typedef typename std::vector< std::vector<vertex> >::size_type size_type;
    {
      // AAA!
      identity_property_map id;
      int n = strong_components(g, &component_no[0], vertex_index_map(id)); 
      components.resize(n);
      for (size_type i = 0; i < num_vertices(g); ++i)
        components[component_no[i]].push_back(i);
    }

#ifdef PRINTS
    cout << "Components are\n" << multiline << components << endl;
#endif

    // Construct a condensation graph 
    // G=(V,E) -> G'=(V',E')
    // V' -- set of strong components in G
    // E' = { (s_1, s_2) : \exists (u \in s_1, v \in s_2) : (u,v) \in E }
    // Note that G' is acyclic
    std::vector< std::vector<vertex> > cg(components.size());
    {
      for (size_type i = 0; i < components.size(); ++i) {
        
        std::vector<vertex> targets;
        {
          for (size_type j = 0; j < components[i].size(); ++j) {
            vertex v = components[i][j];

	    adjacency_iterator k, k_end;
	    for (tie(k, k_end) = adjacent_vertices(v, g); k != k_end; ++k) {
              vertex t = component_no[*k];
              if (t != i) // Avoid loops in the condensation graph
                targets.push_back(t);
            }
          }
        }
        sort(targets.begin(), targets.end());
        std::vector<vertex>::iterator 
	  di = std::unique(targets.begin(), targets.end());
        if (di != targets.end())
          targets.erase(di, targets.end());        
        
        cg[i] = targets;      
      }
    }

#ifdef PRINTS
    cout << "Condensation graph is\n" << multiline << cg << endl;
#endif

    detail::acyclic_closure(cg);

#ifdef PRINTS
    cout << "Its transitive closure is\n" << multiline << cg << endl;
#endif

    // Create needed edges in original graph.
    // The complexity of the following code is due to fact that we don't
    // want *reflexive* transitive closure to be computed, but rather
    // ordinary one.    
    
    std::vector<vertex> looped_vertices;
    {
      vertex_iterator vb, ve;
      for (boost::tie(vb, ve) = vertices(g); vb != ve; ++vb) {  
        adjacency_iterator ab, ae;
        for (boost::tie(ab, ae) = adjacent_vertices(*vb, g); ab != ae; ++ab)
          if (*vb == *ab)
            looped_vertices.push_back(*vb);       
      }
    }

    remove_edge_if(truth<edge>(), g);

    for (size_type i = 0; i < cg.size(); ++i)
      for (size_type j = 0; j < cg[i].size(); ++j) {
        vertex s = i;
        vertex t = cg[i][j];
        for (size_type k = 0; k < components[s].size(); ++k)
          for (size_type l = 0; l < components[t].size(); ++l)
            add_edge(components[s][k], components[t][l], g);
      }
  
    // Since cg's closure is not reflexive closure, we need to process SCC's
    for (size_type i = 0; i < components.size(); ++i)
      if (components[i].size() > 1)
        for (size_type k = 0; k < components[i].size(); ++k)
          for (size_type l = 0; l < components[i].size(); ++l)
            add_edge(components[i][k], components[i][l], g);

    for (size_type i = 0; i < looped_vertices.size(); ++i) {
      vertex v = looped_vertices[i];
      if (components[component_no[v]].size() == 1)
        add_edge(v, v, g);
    }
  }

  template<class G>
  void warshall_transitive_closure(G& g)
  {
    using namespace boost;
    typedef typename graph_traits<G>::vertex_descriptor vertex;
    typedef typename graph_traits<G>::vertex_iterator vertex_iterator;

    function_requires< AdjacencyMatrixConcept<G> >();
    function_requires< MutableGraphConcept<G> >();

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
    function_requires< MutableGraphConcept<G> >();

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





