// (C) Copyright Jeremy Siek 2001. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_GRAPH_ISOMORPHISM_HPP
#define BOOST_GRAPH_ISOMORPHISM_HPP

// UNDER CONSTRUCTION

#include <boost/graph/detail/set_adaptor.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/detail/permutation.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/pending/integer_range.hpp>
#include <boost/limits.hpp>

namespace boost {

  //===========================================================================
  //A simple but slow implementation based on description of the
  //direct isomorphism algorithm in "The Graph Isomorphism Problem" by
  //Scott Fortin and the description in "Combinatorial Algorithms:
  //Theory and Practice" by Reingold, Nievergelt, and Deo. The ideas
  //behind the algorithm were independently invented by Sussenguth,
  //E. H., Jr., "A Graph Theoretic Algorithm for Matching Chemical
  //Structures," J. Chem. Doc., 5 (1965), 36-43 and Unger, S. H, "GIT
  //- a Heuristic Program for Testing Pairs of Directed Line Graphs
  //for Isomorphism," Comm. ACM, 7 (1964), 26-34.
  //
  // The plan is to eventually replace this with the faster algorithm
  // used in "nauty" by Brendan McKay, the beginnings of which is at
  // the bottom of this file.

  struct degree_vertex_invariant {
    template <typename Graph> struct result {
      typedef typename graph_traits<Graph>::degree_size_type type;
    };

    template <typename Graph>
    typename graph_traits<Graph>::degree_size_type
    operator()(typename graph_traits<Graph>::vertex_descriptor v,
               const Graph& g)
    {
      return out_degree(v, g);
    }
  };

  namespace detail {
    
    template <typename InvarMap, typename MultMap>
    struct compare_invariant_multiplicity_predicate
    {
      compare_invariant_multiplicity_predicate(InvarMap i, MultMap m)
        : m_invar(i), m_mult(m) { }

      template <typename Vertex>
      bool operator()(const Vertex& x, const Vertex& y) const {
        return m_mult[m_invar[x]] < m_mult[m_invar[x]];
      }

      InvarMap m_invar;
      MultMap m_mult;
    };
    template <typename InvarMap, typename MultMap>
    compare_invariant_multiplicity_predicate<InvarMap, MultMap>
    compare_invariant_multiplicity(InvarMap i, MultMap m) {
      return compare_invariant_multiplicity_predicate<InvarMap, MultMap>(i,m);
    }

    template <class VertexIndexMap, class Graph>
    struct isomorph_edge_ordering_predicate {
      isomorph_edge_ordering_predicate(VertexIndexMap vip,
                                       const Graph& g)
        : m_index_map(vip), m_g(g) { }
      template <class Edge>
      bool operator()(const Edge& e1, const Edge& e2) const {
        return std::max(get(m_index_map, source(e1, m_g)), 
                        get(m_index_map, target(e1, m_g)))
          < std::max(get(m_index_map, source(e2, m_g)), 
                     get(m_index_map, target(e2, m_g)));
      }
      VertexIndexMap m_index_map;
      const Graph& m_g;
    };
    template <class VertexIndexMap, class G>
    inline isomorph_edge_ordering_predicate<VertexIndexMap,G>
    isomorph_edge_ordering(VertexIndexMap vip, const G& g)
    {
      return isomorph_edge_ordering_predicate<VertexIndexMap,G>(vip, g);
    }

    template <class VertexIter, class EdgeIter, class Graph1, class Graph2,
      class V1IndexMap, class V2IndexMap, class IndexMapping, 
      class Invar1, class Invar2, class Set>
    bool isomorph(VertexIter k, VertexIter last,
                  EdgeIter edge_iter, EdgeIter edge_iter_end,
                  const Graph1& g1, const Graph2& g2,
                  V1IndexMap v1_index_map,
                  V2IndexMap v2_index_map,
                  IndexMapping f, Invar1 invar1, Invar2 invar2,
                  const Set& not_in_S)
    {
      typedef typename graph_traits<Graph2>::vertex_descriptor v2_desc_t;
      if (k == last) 
        return true;

      std::vector<v2_desc_t> my_f_vec(num_vertices(g1));
      typedef typename std::vector<v2_desc_t>::iterator vec_iter;
      iterator_property_map<vec_iter,  V1IndexMap, v2_desc_t, v2_desc_t&>
	my_f(my_f_vec.begin(), v1_index_map);

      typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
      for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
        my_f[*i1] = f[*i1];
      
      // Collect the potential vertices
      std::size_t k_id = get(v1_index_map, *k);
      std::vector<v2_desc_t> vertex_set;
      std::copy(not_in_S.begin(), not_in_S.end(), 
                std::back_inserter(vertex_set));

      // Weed out vertices that do not have the same connectivity as k
      for (; edge_iter != edge_iter_end 
             && (k_id == std::max(get(v1_index_map, source(*edge_iter, g1)),
                                  get(v1_index_map, target(*edge_iter, g1))));
           ++edge_iter) {
        std::vector<v2_desc_t> tmp, adj;
        if (k_id == get(v1_index_map, source(*edge_iter, g1))) {
          v2_desc_t v = f[target(*edge_iter, g1)];
          typename graph_traits<Graph2>::out_edge_iterator ei, ei_end;
          for (tie(ei, ei_end) = out_edges(v, g2); ei != ei_end; ++ei)
            if (invar1[*k] == invar2[target(*ei, g2)])
              adj.push_back(target(*ei, g2));
          std::set_intersection(adj.begin(), adj.end(),
                                vertex_set.begin(), vertex_set.end(),
                                std::back_inserter(tmp),
                                not_in_S.key_comp());
        } else {
          v2_desc_t v = f[source(*edge_iter, g1)];
          typename graph_traits<Graph2>::in_edge_iterator ei, ei_end;
          for (tie(ei, ei_end) = in_edges(v, g2); ei != ei_end; ++ei)
            if (invar1[*k] == invar2[source(*ei, g2)])
              adj.push_back(source(*ei, g2));
          std::set_intersection(adj.begin(), adj.end(),
                                vertex_set.begin(), vertex_set.end(),
                                std::back_inserter(tmp), 
                                not_in_S.key_comp());
        }
        std::swap(vertex_set, tmp);
        if (vertex_set.empty())
          break;
      }
      for (std::size_t j = 0; j < vertex_set.size(); ++j) {
        my_f[*k] = vertex_set[j];
        Set my_not_in_S(not_in_S);
        set_remove(my_not_in_S, vertex_set[j]);
        if (isomorph(boost::next(k), last, edge_iter, edge_iter_end, g1, g2, 
                     v1_index_map, v2_index_map, 
                     my_f, invar1, invar2, my_not_in_S)) {
          for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
            f[*i1] = my_f[*i1];
          return true;
        }
      }
      return false;
    }

  } // namespace detail

  template <typename Graph1, typename Graph2, 
            typename IndexMapping, typename VertexInvariant,
            typename V1Map, typename V2Map>
  bool isomorphism(const Graph1& g1,
		   const Graph2& g2,
		   IndexMapping f,
		   VertexInvariant invariant,
		   V1Map v1_index_map,
		   V2Map v2_index_map)
  {
    typedef typename graph_traits<Graph1>::vertices_size_type size_type;
    typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
    typename graph_traits<Graph2>::vertex_iterator i2, i2_end;
    
    // Quick return with false if |V1| != |V2|
    if (num_vertices(g1) != num_vertices(g2))
      return false;
    
    typedef typename VertexInvariant::template result<Graph1>::type
      InvarValue1;
    typedef typename VertexInvariant::template result<Graph2>::type
      InvarValue2;
    typedef std::vector<InvarValue1> invar_vec1_t;
    typedef std::vector<InvarValue2> invar_vec2_t;
    invar_vec1_t invar1_vec(num_vertices(g1));
    invar_vec2_t invar2_vec(num_vertices(g2));
    typedef typename invar_vec1_t::iterator vec1_iter;
    typedef typename invar_vec2_t::iterator vec2_iter;
    iterator_property_map<vec1_iter, V1Map, InvarValue1, InvarValue1&>
      invar1(invar1_vec.begin(), v1_index_map);
    iterator_property_map<vec2_iter, V2Map, InvarValue2, InvarValue2&>
      invar2(invar2_vec.begin(), v2_index_map);
    
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
      invar1[*i1] = invariant(*i1, g1);
    
    for (tie(i2, i2_end) = vertices(g2); i2 != i2_end; ++i2)
      invar2[*i2] = invariant(*i2, g2);
    
    { // check if the graph's invariants do not match
      invar_vec1_t invar1_tmp(invar1_vec);
      invar_vec2_t invar2_tmp(invar2_vec);
      std::sort(invar1_tmp.begin(), invar1_tmp.end());
      std::sort(invar2_tmp.begin(), invar2_tmp.end());
      if (! std::equal(invar1_tmp.begin(), invar1_tmp.end(), 
		       invar2_tmp.begin()))
	return false;
    }
    
    // compute invariant multiplicity
    std::vector<std::size_t> invar_mult(num_vertices(g1), 0);
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)      
      ++invar_mult[invar1[*i1]];
    
    // calculate the permutation that would sort vertices based on
    // invariant multiplicity
    std::vector<size_type> perm;
    integer_range<size_type> range(0, num_vertices(g1));
    std::copy(range.begin(), range.end(), std::back_inserter(perm));
    std::sort(perm.begin(), perm.end(),
	      detail::compare_invariant_multiplicity(invar1_vec.begin(),
						     invar_mult.begin()));
    
    typedef typename graph_traits<Graph1>::vertex_descriptor VertexG1;
    std::vector<VertexG1> g1_vertices;
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
      g1_vertices.push_back(*i1);
    permute(g1_vertices.begin(), g1_vertices.end(), perm.begin());
    
    typedef typename graph_traits<Graph1>::edge_descriptor edge1_t;
    std::vector<edge1_t> edge_set;
    std::copy(edges(g1).first, edges(g1).second, 
	      std::back_inserter(edge_set));
    
    std::sort(edge_set.begin(), edge_set.end(), 
	      detail::isomorph_edge_ordering
	      (make_iterator_property_map(perm.begin(), v1_index_map, perm[0]), g1));
    
    typename std::vector<VertexG1>::iterator first = g1_vertices.begin();
    typename graph_traits<Graph2>::vertex_iterator vi, vi_end;
    for (tie(vi, vi_end) = vertices(g2); vi != vi_end; ++vi) {
      f[*first] = *vi;
      
      typedef typename graph_traits<Graph1>::vertex_descriptor VertexG2;
      typedef typename property_traits<V2Map>::value_type V2Idx;
      typedef indirect_cmp<V2Map, std::less<V2Idx> >  Cmp;
      Cmp cmp(v2_index_map);
      std::set<VertexG2, Cmp> not_in_S(cmp);
      for (tie(i2, i2_end) = vertices(g2); i2 != i2_end; ++i2)
	set_insert(not_in_S, *i2);
      set_remove(not_in_S, *vi);
      
      if(detail::isomorph
	 (boost::next(first), g1_vertices.end(), 
	  edge_set.begin(), edge_set.end(), 
	  g1, g2,
	  make_iterator_property_map(perm.begin(), v1_index_map, perm[0]),
	  v2_index_map, f, invar1, invar2, not_in_S))
	return true;
    }
    return false;
  }

  template <typename Graph1, typename Graph2, class P, class T, class R>
  bool isomorphism(const Graph1& g1,
                   const Graph2& g2,
                   const bgl_named_params<P,T,R>& params)
  {
    typedef typename graph_traits<Graph2>::vertex_descriptor v2_desc_t;
    typename std::vector<v2_desc_t>::size_type
      n = is_default_param(get_param(params, vertex_isomorphism_t()))
        ? num_vertices(g1) : 1;
    std::vector<v2_desc_t> f(n);
    degree_vertex_invariant default_invar;
    return isomorphism
      (g1, g2, 
       choose_param
       (get_param(params, vertex_isomorphism_t()),
        make_iterator_property_map
        (f.begin(), 
         choose_const_pmap(get_param(params, vertex_index1),
                     g1, vertex_index), f[0])),
       choose_param(get_param(params, vertex_invariant_t()),
                    default_invar),
       choose_const_pmap(get_param(params, vertex_index1),
                   g1, vertex_index),
       choose_const_pmap(get_param(params, vertex_index2),
                   g2, vertex_index)
       );
  }

  template <typename Graph1, typename Graph2>
  bool isomorphism(const Graph1& g1, const Graph2& g2)
  {
    typedef typename graph_traits<Graph1>::vertices_size_type size_type;
    typedef typename graph_traits<Graph2>::vertex_descriptor v2_desc_t;
    std::vector<v2_desc_t> f(num_vertices(g1));
    degree_vertex_invariant invariant;
    return isomorphism
      (g1, g2,
       make_iterator_property_map(f.begin(), get(vertex_index, g1), f[0]),
       invariant,
       get(vertex_index, g1),
       get(vertex_index, g2)
       );
  }

#if 0

  // The beginnings of a C++ implementation of canonical labelling
  // derived from the nauty program.

  namespace detail {

    //=========================================================================
    // Mathematical Set Concept
    //
    // bool set_equal(a, b)
    // bool is_subset(a, b)
    // bool is_proper_subset(a, b)
    // void set_intersect(a, b, c)
    // void set_union(a, b, c)
    // void set_difference(a, b, c)
    // void set_complement(a, c)
    // void set_insert(a, x)
    // void set_remove(a, x)
    // bool set_contains(a, x)
    // void set_clear(a)
    // bool set_empty(a)
    // set_traits<S>::size_type
    // size_type set_size(a)
    // set_traits<S>::iterator
    // iterator set_begin(a)
    // iterator set_end(a)

    //=========================================================================
    // d_G(v,W) == out_degree(v, vertex_subset_filter(W, G))
    //
    // for all V_i, V_j \in \pi
    //  d_G(v_i,V_j) = d_ij for all v_i \in V_i

    template <typename PartitionCells, typename Graph>
    bool is_equitable_partition(const PartitionCells& p,
                                const Graph& g)
    {
      typedef typename PartitionCells::value_type cell_t;
      typedef typename vertex_subset_filter<Graph, cell_t>::type subgraph_t;
      for (i = 0; i != size(p); ++i)
        for (j = 0; j != size(p); ++j) {
          subgraph_t w(p[j], g);
          assert(size(p[i]) > 0);
          vi = begin(p[i]);
          d = out_degree(*vi++, w);
          for (; vi != end(p[i]); ++vi) {
            if (d != out_degree(*vi, w))
              return false;
          }
        }
      return true;
    }

    //=========================================================================
    template <typename Range, typename Set2, typename Permutation>
    void permutation_set(const Range& set1, 
                         Set2& set2, 
                         const Permutation& perm)
    {
      set_clear(set2);
      for (typename container_traits<Range>::iterator i = begin(set1);
           i != end(set1); ++i)
        set_insert(set2, perm[*i]);
    }
    
    //=========================================================================
    template <typename Graph, typename Permutation>
    bool is_automorphism(const Graph& g, const Permutation& perm)
    {
      for (tie(i, i_end) = vertices(g); i != i_end; ++i)
        for (tie(j, j_end) = adjacent_vertices(*i, g); j != j_end; ++j)
          if (!edge(*j, perm[*j], g).second)
            return false;
      return true;
    }

    //=========================================================================
    template <typename OrbitsMap, typename Permutation>
    int join_orbits(OrbitsMap& orbits, 
                    const Permutation& perm, 
                    int n)
    {
      int i, j1, j2;
      for (i = 0; i < n; ++i) {
        j1 = orbits[i];
        while (orbits[j1] != j1)
          j1 = orbits[j1];
        j2 = orbits[perm[i]];
        while (orbits[j2] != j2)
          j2 = orbits[j2];
        
        if (j1 < j2)
          orbits[j2] = j1;
        else if (j1 > j2)
          orbits[j1] = j2;
      }
      j1 = 0;
      for (i = 0; i < n; ++i)
        if ((orbits[i] = orbits[orbits[i]]) == i)
          ++j1;
      return j1;
    }



    //=========================================================================
    template <typename Graph1, typename Graph2, 
              typename Labeling1, typename Labeling2>
    int test_canonical_labeling(const Graph1& g, const Graph2& canonical_g,
                                const Labeling1& g_labeling, 
                                const Labeling2& canonical_g_labeling, 
                                int& same_rows)
    {
      int i, j;
      set *ph;
      
      permutation_type work_perm(n);
      set_type work_i_set(m), canon_i_set(m);
      
      for (i = 0; i < n; ++i)
        work_perm[labeling[i]] = i;

      //for (i = 0, ph = canong; i < n; ++i, ph += M) {
      for (tie(i, i_end) = vertices(canonical_g); i != i_end; ++i) {
        
        permutation_set(adjacent_vertices(g_labeling[*i], g),
                        work_i_set, work_perm);

        // Or perhaps canonical_g should be stored with ordered out-edges
        permutation_set(adjacent_vertices(*i, canonical_g),
                        canon_i_set, canonical_g_labeling);
        
        int cmp = set_lex_compare(work_i_set, canon_i_set);

        if (cmp != 0) {
          same_rows = i;
          return cmp;
        }
      }
      same_rows = n;
      return 0;
    }
    
    
    template <typename Graph, ...>
    class canonical_labeling_algo {
      typedef std::vector<int> nvector_type;
    public:

      //=======================================================================
      int first_path_node(nvector_type& lab, 
                          nvector_type& ptn, 
                          int level, 
                          int numcells, 
                          tcell_list_type::iterator tcnode_parent)
      {
        tcell_list_type::iterator tcnode_this = tcnode_parent;
        ++tcnode_this;
        if (tcnode_this == tcell_list.end()) {
          tcell_list.push_back(set_type());
          tcnode_this = prior(tcell_list.end());
        }
        set_type& tcell = *tcnode_this;
        
        // doref()
        refine_partition(g, lab, level, numcells, qinv, workperm,
                         active, refcode, refinement_fun, vertex_invariant_fun,
                         mininvarlevel, maxinvarlevel, invararg, 
                         digraph, M, n);

        firstcode[level] = refcode;
        if (qinvar > 0) {
          ++invsuccesses;
          if (mininvarlevel < 0)
            mininvarlevel = level;
          if (maxinvarlevel < 0)
            maxinvarlevel = level;
          if (level < invarsuclevel)
            invarsuclevel = level;
        }
        tc = -1;
        if (numcells != n) {
          /* locate new target cell, setting tc to its position in lab, tcell
             to its contents, and tcellsize to its size: */
          target_cell_fun(g, lab, ptn, level, numcells, tcell,&tcellsize,
                    &tc, tc_level, -1, M, n);
        }
        firsttc[level] = tc;

    /* optionally call user-defined node examination procedure: */
        tree_node_fun(g,lab,ptn,level,numcells,tc,(int)firstcode[level],M,n);

        if (numcells == n)      /* found first leaf? */
        {
            first_terminal(lab,level);
            level_fun(lab,ptn,level,orbits,stats,0,1,1,n,0,n);
            return level-1;
        }

        if (noncheaplevel >= level && !cheapautom(ptn,level,digraph,n))
            noncheaplevel = level + 1;

    /* use the elements of the target cell to produce the children: */
        index = 0;
        for (tv1 = tv = nextelement(tcell,M,-1); tv >= 0;
             tv = nextelement(tcell,M,tv))
        {
          if (orbits[tv] == tv)   /* ie, not equiv to previous child */
            {
              breakout(lab,ptn,level+1,tc,tv,active,M);
              insert(tv, fixedpts);
              cosetindex = tv;
              if (tv == tv1) {
                  rtnlevel = first_path_node(lab,ptn,level+1,numcells+1,
                                             tcnode_this);
                  childcount = 1;
                  gca_first = level;
                  stabvertex = tv1;
              }
              else {
                rtnlevel = other_node(lab,ptn,level+1,numcells+1,
                                      tcnode_this);
                ++childcount;
              }
              remove(tv, fixedpts);
              if (rtnlevel < level)
                return rtnlevel;
              if (needshortprune) {
                needshortprune = FALSE;
                shortprune(tcell,fmptr-M,M);
              }
              recover(ptn,level);
            }
          if (orbits[tv] == tv1)  /* ie, in same orbit as tv1 */
            ++index;
        }
        // MULTIPLY(stats->grpsize1, stats->grpsize2, index);
        
        if (tcellsize == index && allsamelevel == level + 1)
          --allsamelevel;
        
        if (domarkers)
          writemarker(level,tv1,index,tcellsize,stats->numorbits,numcells);
        level_fun(lab,ptn,level,orbits,stats,tv1,index,tcellsize,
                      numcells,childcount,n);
        return level-1;
      } // first_path_node()


      //=======================================================================
      int other_node(lab, ptn, level, numcells, tcnode_parent)
      {
        int tv;
        int tv1,refcode,rtnlevel,tcellsize,tc,qinvar;
        short code;
        tcell_list_type::iterator tcnode_this = tcnode_parent;
        ++tcnode_type;
        if (tcnode_this == tcell_list.end()) {
          tcell_list.push_back(set_type());
          tcnode_this = prior(tcell_list.end());
        }
        set_type& tcell = *tcnode_this;
      
        ++stats->numnodes;
      
        // doref()
        refine_partition(g,lab,ptn,level,numcells,qinvar,workperm,active,
                         refcode,refinement_fun,vertex_invariant_fun,
                         mininvarlevel,maxinvarlevel,
                         invararg,digraph,M,n);
        code = (short)refcode;
        if (qinvar > 0) {
          ++invapplics;
          if (qinvar == 2) {
            ++invsuccesses;
            if (level < invarsuclevel)
              invarsuclevel = level;
          }
        }

        if (eqlev_first == level - 1 && code == firstcode[level])
          eqlev_first = level;
        if (getcanon) {
          if (eqlev_canon == level - 1){
            if (code < canoncode[level])
              comp_canon = -1;
            else if (code > canoncode[level])
              comp_canon = 1;
            else {
              comp_canon = 0;
              eqlev_canon = level;
            }
          }
          if (comp_canon > 0)
            canoncode[level] = code;
        }

        tc = -1;

        // If children will be required, find new target cell and set
        // tc to its position in lab, tcell to its contents, and
        // tcellsize to its size:

        if (numcells < n && (eqlev_first == level ||
                             getcanon && comp_canon >= 0)) {
          if (!getcanon || comp_canon < 0) {
            target_cell_fun(g,lab,ptn,level,numcells,tcell,tcellsize,
                      tc,tc_level,firsttc[level],M,n);
            if (tc != firsttc[level])
              eqlev_first = level - 1;
          }
          else
            target_cell_fun(g,lab,ptn,level,numcells,tcell,tcellsize,
                      tc,tc_level,-1,M,n);
          stats->tctotal += tcellsize;
        }

        // optionally call user-defined node examination procedure: 
        tree_node_fun(g,lab,ptn,level,numcells,tc,(int)code,M,n);

        // call process_node to classify the type of this node:

        rtnlevel = process_node(lab,ptn,level,numcells);
        if (rtnlevel < level)   /* keep returning if necessary */
          return rtnlevel;
        if (needshortprune) {
          needshortprune = FALSE;
          shortprune(tcell,fmptr-M,M);
        }
      
        if (!cheapautom(ptn,level,digraph,n))
          noncheaplevel = level + 1;
      
        // use the elements of the target cell to produce the children:
        for (tv1 = tv = nextelement(tcell,M,-1); tv >= 0;
             tv = nextelement(tcell,M,tv)) {
          breakout(lab,ptn,level+1,tc,tv,active,M);
          insert(tv, fixedpts);
          rtnlevel = other_node(lab,ptn,level+1,numcells+1,tcnode_this);

          remove(tv, fixedpts);
        
          if (rtnlevel < level)
            return rtnlevel;
          // use stored automorphism data to prune target cell:
          if (needshortprune) {
            needshortprune = FALSE;
            short_prune(tcell,fmptr-M,M);
          }
          if (tv == tv1)
            long_prune(tcell,fixedpts,workspace,fmptr,M);
        
          recover(ptn,level);
        }
        return level-1;
      }
      
      //=======================================================================
      void first_terminal(nvector_type& lab, int level)
      {
        register int i;
        
        stats->maxlevel = level;
        gca_first = allsamelevel = eqlev_first = level;
        firstcode[level+1] = 077777;
        firsttc[level+1] = -1;
        
        for (i = 0; i < n; ++i)
          firstlab[i] = lab[i];
        
        if (getcanon) {
          canonlevel = eqlev_canon = gca_canon = level;
          comp_canon = 0;
          samerows = 0;
          for (i = 0; i < n; ++i)
            canonlab[i] = lab[i];
          for (i = 0; i <= level; ++i)
            canoncode[i] = firstcode[i];
          canoncode[level+1] = 077777;
          stats->canupdates = 1;
        }
      }
    
      //=======================================================================
     int process_node(nvector_type& lab, nvector_type& ptn, 
                      int level, int numcells)
     {
       int i,code,save,newlevel;
       bool ispruneok;
       int sr;

       code = 0;
       if (eqlev_first != level && (!getcanon || comp_canon < 0))
         code = 4;
       else if (numcells == n) {
         if (eqlev_first == level) {
           for (i = 0; i < n; ++i)
             workperm[firstlab[i]] = lab[i];
           
           if (gca_first >= noncheaplevel ||
               isautom(g,workperm,digraph,M,n))
             code = 1;
         }
         if (code == 0)
           if (getcanon) {
             sr = 0;
             if (comp_canon == 0) {
               if (level < canonlevel)
                 comp_canon = 1;
               else {
                 update_can(g,canong,canonlab, samerows,M,n);
                 samerows = n;
                 comp_canon = testcanlab(g,canong,lab,&sr,M,n);
               }
             }
             if (comp_canon == 0) {
               for (i = 0; i < n; ++i)
                 workperm[canonlab[i]] = lab[i];
               code = 2;
             }
             else if (comp_canon > 0)
               code = 3;
             else
               code = 4;
           }
           else
             code = 4;
       }

       if (code != 0 && level > stats->maxlevel)
         stats->maxlevel = level;
       
       switch (code) {
       case 0:                 /* nothing unusual noticed */
         return level;
         
       case 1:                 /* lab is equivalent to firstlab */
         if (fmptr == worktop)
           fmptr -= 2 * M;
         fmperm(workperm,fmptr,fmptr+M,M,n);
         fmptr += 2 * M;
         if (writeautoms)
           writeperm(outfile,workperm,cartesian,linelength,n);
         stats->numorbits = orbjoin(orbits,workperm,n);
         ++stats->numgenerators;
         automorphism_fun(stats->numgenerators,workperm,orbits,
                       stats->numorbits,stabvertex,n);
         return gca_first;
         
       case 2:                 /* lab is equivalent to canonlab */
         if (fmptr == worktop)
           fmptr -= 2 * M;
         fmperm(workperm,fmptr,fmptr+M,M,n);
         fmptr += 2 * M;
         save = stats->numorbits;
         stats->numorbits = orbjoin(orbits,workperm,n);
         if (stats->numorbits == save)
           {
             if (gca_canon != gca_first)
               needshortprune = TRUE;
             return gca_canon;
           }
         if (writeautoms)
           writeperm(outfile,workperm,cartesian,linelength,n);
         ++stats->numgenerators;
         automorphism_fun(stats->numgenerators,workperm,orbits,
                       stats->numorbits,stabvertex,n);
         if (orbits[cosetindex] < cosetindex)
           return gca_first;
         if (gca_canon != gca_first)
           needshortprune = TRUE;
         return gca_canon;
         
       case 3:                 /* lab is better than canonlab */
         ++stats->canupdates;
         for (i = 0; i < n; ++i)
           canonlab[i] = lab[i];
         canonlevel = eqlev_canon = gca_canon = level;
         comp_canon = 0;
         canoncode[level+1] = 077777;
         samerows = sr;
         break;
         
       case 4:                /* non-automorphism terminal node */
         ++stats->numbadleaves;
         break;
       }  /* end of switch statement */
       
       /* only cases 3 and 4 get this far: */
       if (level != noncheaplevel) {
         ispruneok = TRUE;
         if (fmptr == worktop)
           fmptr -= 2 * M;
         fmptn(lab,ptn,noncheaplevel,fmptr,fmptr+M,M,n);
         fmptr += 2 * M;
       }
       else
         ispruneok = FALSE;
       
       save = (allsamelevel > eqlev_canon ? allsamelevel-1 : eqlev_canon);
       newlevel = (noncheaplevel <= save ? noncheaplevel-1 : save);
       
       if (ispruneok && newlevel != gca_first)
         needshortprune = TRUE;
       return newlevel;
     } // process_node()

      //=======================================================================
      void recover(nvector_type& ptn, int level)
      {
        int i;

        for (i = 0; i < n; ++i)
          if (ptn[i] > level)
            ptn[i] = INFINITY;
        
        if (level < noncheaplevel)
          noncheaplevel = level + 1;
        if (level < eqlev_first)
          eqlev_first = level;
        if (getcanon) {
          if (level < gca_canon)
            gca_canon = level;
          if (level <= eqlev_canon) {
            eqlev_canon = level;
            comp_canon = 0;
          }
        }
      }

      //=======================================================================
      void write_marker(int level, int tv, int index, int tcellsize,
                        int numorbits, int numcells)
      {

        outfile << "level ";
        outfile << level;
        outfile << ":  ";
        if (numcells != numorbits) {
          outfile << numcells;
          outfile << " cell";
          if (numcells == 1)
            outfile << "; ";
          else
            outfile << "s; ";
        }
        outfile << numorbits;
        outfile << " orbit";
        if (numorbits == 1)
          outfile << "; ";
        else
          outfile << "s; ";
        outfile << tv+labelorg;
        outfile << " fixed; index ";
        outfile << index;
        if (tcellsize != index) {
          outfile << "/";
          outfile << tcellsize;
        }
        outfile << "\n";
      }

      //=======================================================================

      

    protected:
      typedef std::bitset<int> set_type;
      typedef std::vector<int> permutation_type;

      typedef std::list<set_type> tcell_list_type;
      tcell_list_type tcell_list; // target cell list

      int m, n;
      nvector_type orbits;
      graph_type canong;
      Graph g;
      long invapplics,invsuccesses;
      int invarsuclevel;
      int gca_first,            // level of greatest common ancestor of current
                                // node and first leaf 
        gca_canon,              // ditto for current node and bsf leaf
        noncheaplevel,          // level of greatest ancestor for which
                                // cheapautom  == FALSE 
        allsamelevel,           // level of least ancestor of first leaf for
                                // which all descendant leaves are known to be
                                // equivalent 
        eqlev_first,            // level to which codes for this node match 
                                // those for first leaf 
        eqlev_canon,   /* level to which codes for this node match those
                          for the bsf leaf. */
        comp_canon,    /* -1,0,1 according as code at eqlev_canon+1 is
                          <,==,> that for bsf leaf.  Also used for
                          similar purpose during leaf processing */
        samerows,      /* number of rows of canong which are correct for
                          the bsf leaf  BDM:correct? */
        canonlevel,    /* level of bsf leaf */
        stabvertex,    /* point fixed in ancestor of first leaf at level
                          gca_canon */
        cosetindex;    /* the point being fixed at level gca_first */
      
      bool needshortprune;       /* used to flag calls to shortprune */

      set_type defltwork;        /* workspace in case none provided */
      permutation_type workperm;   /* various scratch uses */
      set_type fixedpts;           /* points which were explicitly
                                        fixed to get current node */
      permutation_type firstlab,   /* label from first leaf */
        canonlab;   /* label from bsf leaf */

      codes_type firstcode,      /* codes for first leaf */
        canoncode;      /* codes for bsf leaf */
      
      std::vector<int> firsttc;     /* index of target cell for left path */
      set_type active;             /* used to contain index to cells now
                                        active for refinement purposes */

#if 0
      set *workspace,*worktop;      /* first and just-after-last addresses of
                                        work area to hold automorphism data */
      set *fmptr;                   /* pointer into workspace */

      int alloc_m = 0;
#endif

      // Various user-parameterizable functions:
      TargetCellFunction target_cell_fun;// target cell function
      RefinementFunction refinement_fun; // refinement function
      AutomorphismFunction automorphism_fun; // automorphism callback
      LevelFunction level_fun; // level callback
      TreeNodeFunction tree_node_fun;   // tree node callback
      VertexInvariantFunction vertex_invariant_fun; // vertex invariant function
    };


    // This is a "UPROC" user-procedure
    class target_cell
    {
    public:
      target_cell() { }

      operator()(Graph& g,
                 nvector_type& lab, nvector_type& ptn,
                 int level, int numcells,
                 set_type& tcell,
                 int& cellpos, int tc_level, int hint, int m, int n)
      {
        int i, j, k;

        if (hint >= 0 && ptn[hint] > level &&
            (hint == 0 || ptn[hint-1] <= level))
          i = hint;
        else if (level <= tc_level)
          i = bestcell(g, lab, ptn, level, tc_level, m, n);
        else
          for (i = 0; i < n && ptn[i] <= level; ++i) 
            { }
        if (i == n)
          i = j = 0;
        else
          for (j = i + 1; ptn[j] > level; ++j) 
            { }
        
        tcell.clear();
        for (k = i; k <= j; ++k)
          tcell.insert(lab[k]);
        
        *cellpos = i;
      }
    };

  } // namespace detail


  template <typename Graph, 
            typename VertexIndexMap1, 
            typename VertexIndexMap2>
  void 
  canonical_labeling(const Graph& g, // IN
                     VertexIndexMap1 old_labels, // IN
                     VertexIndexMap2 new_labels, // OUT
                     OrbitMap orbits) // OUT
  {
    typedef typename property_traits<VertexIndexMap2>::value_type label_type;

    std::stack<partition_type> S;

    while (! S.empty() {
      
      
      
    }

  }
#endif

} // namespace boost

#endif // BOOST_GRAPH_ISOMORPHISM_HPP
