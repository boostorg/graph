// (C) Copyright Jeremy Siek 2001. Permission to copy, use, modify,
// sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

// See http://www.boost.org/libs/graph/doc/isomorphism-impl.pdf 
// for a description of the implementation of the isomorphism function
// defined in this header file.

#ifndef BOOST_GRAPH_ISOMORPHISM_HPP
#define BOOST_GRAPH_ISOMORPHISM_HPP

#include <algorithm>
#include <boost/graph/detail/set_adaptor.hpp>
#include <boost/pending/indirect_cmp.hpp>
#include <boost/graph/detail/permutation.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/graph_concepts.hpp>
#include <boost/property_map.hpp>
#include <boost/pending/integer_range.hpp>
#include <boost/limits.hpp>
#include <boost/static_assert.hpp>
#include <boost/graph/depth_first_search.hpp>

namespace boost {

  struct degree_vertex_invariant {
    template <typename Graph> struct result {
      typedef typename graph_traits<Graph>::degree_size_type type;
    };
    template <typename Graph>
    typename graph_traits<Graph>::degree_size_type
    operator()(typename graph_traits<Graph>::vertex_descriptor v, const Graph& g)
      { return out_degree(v, g); }
  };
  

  namespace detail {
    template <class VertexIter, class EdgeIter, class Graph1, class Graph2,
      class IndexMap1, class IndexMap2, class IndexMapping, 
      class Invar1, class Invar2, class Set>
    bool isomorph(VertexIter k_iter, VertexIter last,
                  EdgeIter edge_iter, EdgeIter edge_iter_end,
                  const Graph1& g1, const Graph2& g2,
                  IndexMap1 index_map1,
                  IndexMap2 index_map2,
                  IndexMapping f, Invar1 invar1, Invar2 invar2,
                  const Set& not_in_S)
    {
      typedef typename graph_traits<Graph1>::vertex_descriptor vertex1_t;
      typedef typename graph_traits<Graph2>::vertex_descriptor vertex2_t;
      typedef typename graph_traits<Graph1>::vertices_size_type size_type;
      
      vertex1_t k = *k_iter;
      
      if (k_iter == last) 
        return true;
      
      std::vector<vertex2_t> my_f_vec(num_vertices(g1));
      typedef typename std::vector<vertex2_t>::iterator vec_iter;
      iterator_property_map<vec_iter,  IndexMap1, vertex2_t, vertex2_t&>
        my_f(my_f_vec.begin(), index_map1);
      
      typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
      for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
        my_f[*i1] = get(f, *i1);
      
      std::vector<vertex2_t> potential_matches;
      bool some_edges = false;
      
      for (; edge_iter != edge_iter_end; ++edge_iter) {
        if (get(index_map1, k) != edge_num(*edge_iter, index_map1, g1))
          break;      
        if (k == source(*edge_iter, g1)) { // (k,j)
          vertex1_t j = target(*edge_iter, g1);
          std::vector<vertex2_t> out;
          typename graph_traits<Graph2>::in_edge_iterator ei, ei_end;
          for (tie(ei, ei_end) = in_edges(get(f, j), g2); ei != ei_end; ++ei) {
            vertex2_t v = source(*ei, g2); // (v,f[j])
            if (invar1[k] == invar2[v] && not_in_S[v])
              out.push_back(v);
          }
          
          if (some_edges == false) {
            indirect_cmp<IndexMap2,std::less<std::size_t> > cmp(index_map2);
            std::sort(out.begin(), out.end(), cmp);
            std::copy(out.begin(), out.end(), std::back_inserter(potential_matches));
            
          } else {
            indirect_cmp<IndexMap2,std::less<std::size_t> > cmp(index_map2);
            std::sort(out.begin(), out.end(), cmp);
            std::vector<vertex2_t> tmp_matches;
            std::set_intersection(out.begin(), out.end(),
                                  potential_matches.begin(), potential_matches.end(),
                                  std::back_inserter(tmp_matches), cmp);
            std::swap(potential_matches, tmp_matches);
          }
          some_edges = true;
        } else { // (j,k)
          vertex1_t j = source(*edge_iter, g1);
          std::vector<vertex2_t> in;
          typename graph_traits<Graph2>::out_edge_iterator ei, ei_end;
          for (tie(ei, ei_end) = out_edges(get(f, j), g2); ei != ei_end; ++ei) {
            vertex2_t v = target(*ei, g2); // (f[j],v)
            if (invar1[k] == invar2[v] && not_in_S[v])
              in.push_back(v);
          }
          
          if (some_edges == false) {
            indirect_cmp<IndexMap2,std::less<std::size_t> > cmp(index_map2);
            std::sort(in.begin(), in.end(), cmp);
            std::copy(in.begin(), in.end(), std::back_inserter(potential_matches));
          } else {
            indirect_cmp<IndexMap2, std::less<std::size_t> > cmp(index_map2);
            std::sort(in.begin(), in.end(), cmp);
            std::vector<vertex2_t> tmp_matches;
            std::set_intersection(in.begin(), in.end(),
                                  potential_matches.begin(), potential_matches.end(),
                                  std::back_inserter(tmp_matches), cmp);
            std::swap(potential_matches, tmp_matches);
          }
          some_edges = true;
        }
        if (potential_matches.empty())
          break;
      } // for edge_iter
      if (some_edges == false) {
        typename graph_traits<Graph2>::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(g2); vi != vi_end; ++vi)
          if (not_in_S[*vi])
            potential_matches.push_back(*vi);
      }
      
      for (std::size_t j = 0; j < potential_matches.size(); ++j) {
        my_f[k] = potential_matches[j];
        std::vector<char> my_not_in_S_vec(num_vertices(g2));
        iterator_property_map<char*, IndexMap2, char, char&>
          my_not_in_S(&my_not_in_S_vec[0], index_map2);
        typename graph_traits<Graph2>::vertex_iterator vi, vi_end;
        for (tie(vi, vi_end) = vertices(g2); vi != vi_end; ++vi)
          my_not_in_S[*vi] = not_in_S[*vi];;
        my_not_in_S[potential_matches[j]] = false;
        
        if (isomorph(boost::next(k_iter), last, edge_iter, edge_iter_end, g1, g2, 
                     index_map1, index_map2, 
                     my_f, invar1, invar2, my_not_in_S)) {
          for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
            put(f, *i1, my_f[*i1]);
          return true;
        }
      }
      return false;
    }
    
  } // namespace detail

  namespace detail {
    template <typename Graph1, typename IndexMap1>
    struct record_dfs_order : public default_dfs_visitor {
      typedef typename graph_traits<Graph1>::vertices_size_type size_type;
      typedef typename graph_traits<Graph1>::vertex_descriptor vertex;
  
      record_dfs_order(std::vector<size_type>& dfs_order, IndexMap1 index) 
        : dfs_order(dfs_order), index(index) { }
  
      void discover_vertex(vertex v, const Graph1& g) const {
        dfs_order.push_back(get(index, v));
      }
      std::vector<size_type>& dfs_order; 
      IndexMap1 index;
    };
  } // namespace detail
  
  namespace detail {
    template <typename InvarMap, typename MultMap>
    struct compare_invariant_multiplicity_predicate
    {
      compare_invariant_multiplicity_predicate(InvarMap i, MultMap m)
        : m_invar(i), m_mult(m) { }
  
      template <typename Vertex>
      bool operator()(const Vertex& x, const Vertex& y) const
        { return m_mult[m_invar[x]] < m_mult[m_invar[x]]; }
  
      InvarMap m_invar;
      MultMap m_mult;
    };
    template <typename InvarMap, typename MultMap>
    compare_invariant_multiplicity_predicate<InvarMap, MultMap>
    compare_invariant_multiplicity(InvarMap i, MultMap m) {
      return compare_invariant_multiplicity_predicate<InvarMap, MultMap>(i,m);
    }
  } // namespace detail
  
  namespace detail {
  
    template <typename VertexIndexMap, typename Graph>
    std::size_t edge_num(const typename graph_traits<Graph>::edge_descriptor e,
                         VertexIndexMap index_map, const Graph& g) {
      return std::max(get(index_map, source(e, g)), get(index_map, target(e, g)));    
    }
  
    template <typename VertexIndexMap, typename Graph>
    class edge_ordering_fun {
    public:
      edge_ordering_fun(VertexIndexMap vip, const Graph& g)
        : m_index_map(vip), m_g(g) { }
      template <typename Edge>
      bool operator()(const Edge& e1, const Edge& e2) const {
        return edge_num(e1, m_index_map, m_g) < edge_num(e2, m_index_map, m_g);
      }
      VertexIndexMap m_index_map;
      const Graph& m_g;
    };
    template <class VertexIndexMap, class G>
    inline edge_ordering_fun<VertexIndexMap,G>
    edge_ordering(VertexIndexMap vip, const G& g)
    {
      return edge_ordering_fun<VertexIndexMap,G>(vip, g);
    }
  } // namespace detail
  

  template <typename Graph1, typename Graph2, 
            typename IndexMapping, typename VertexInvariant,
            typename IndexMap1, typename IndexMap2>
  bool isomorphism(const Graph1& g1, const Graph2& g2, 
                   IndexMapping f, VertexInvariant invariant,
                   IndexMap1 index_map1, IndexMap2 index_map2)
  {
    typedef typename graph_traits<Graph1>::vertex_descriptor vertex1_t;
    typedef typename graph_traits<Graph2>::vertex_descriptor vertex2_t;
    typedef typename graph_traits<Graph1>::vertices_size_type size_type;
    typename graph_traits<Graph1>::vertex_iterator i1, i1_end;
    typename graph_traits<Graph2>::vertex_iterator i2, i2_end;
    
    // Graph requirements
    function_requires< VertexListGraphConcept<Graph1> >();
    function_requires< EdgeListGraphConcept<Graph1> >();
    function_requires< VertexListGraphConcept<Graph2> >();
    function_requires< BidirectionalGraphConcept<Graph2> >();
    
    // Property map requirements
    function_requires< ReadWritePropertyMapConcept<IndexMapping, vertex1_t> >();
    typedef typename property_traits<IndexMapping>::value_type IndexMappingValue;
    BOOST_STATIC_ASSERT((is_same<IndexMappingValue, vertex2_t>::value));
    
    function_requires< ReadablePropertyMapConcept<IndexMap1, vertex1_t> >();
    typedef typename property_traits<IndexMap1>::value_type IndexMap1Value;
    BOOST_STATIC_ASSERT((is_convertible<IndexMap1Value, size_type>::value));
    
    function_requires< ReadablePropertyMapConcept<IndexMap2, vertex2_t> >();
    typedef typename property_traits<IndexMap2>::value_type IndexMap2Value;
    BOOST_STATIC_ASSERT((is_convertible<IndexMap2Value, size_type>::value));
    
    if (num_vertices(g1) != num_vertices(g2))
      return true;
    
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
    iterator_property_map<vec1_iter, IndexMap1, InvarValue1, InvarValue1&>
      invar1(invar1_vec.begin(), index_map1);
    iterator_property_map<vec2_iter, IndexMap2, InvarValue2, InvarValue2&>
      invar2(invar2_vec.begin(), index_map2);
    
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
    
        std::vector<std::size_t> invar_mult(num_vertices(g1), 0);
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)      
      ++invar_mult[invar1[*i1]];
    
    std::vector<size_type> perm;
    integer_range<size_type> range(0, num_vertices(g1));
    std::copy(range.begin(), range.end(), std::back_inserter(perm));
    std::sort(perm.begin(), perm.end(),
              detail::compare_invariant_multiplicity(invar1_vec.begin(),
                                                     invar_mult.begin()));
    
    std::vector<vertex1_t> g1_vertices;
    for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
      g1_vertices.push_back(*i1);
    permute(g1_vertices.begin(), g1_vertices.end(), perm.begin());
    
    {
      perm.clear();
            std::vector<default_color_type> color_vec(num_vertices(g1));
      for (typename std::vector<vertex1_t>::iterator ui = g1_vertices.begin();
           ui != g1_vertices.end(); ++ui) {
        if (color_vec[get(index_map1, *ui)] 
            == color_traits<default_color_type>::white()) {
          depth_first_visit
            (g1, *ui, detail::record_dfs_order<Graph1, IndexMap1>(perm, 
                                                             index_map1), 
             make_iterator_property_map(&color_vec[0], index_map1, 
                                        color_vec[0]));
        }
      }
      
      g1_vertices.clear();
      for (tie(i1, i1_end) = vertices(g1); i1 != i1_end; ++i1)
        g1_vertices.push_back(*i1);
      permute(g1_vertices.begin(), g1_vertices.end(), perm.begin());
    }
    
    typedef typename graph_traits<Graph1>::edge_descriptor edge1_t;
    std::vector<edge1_t> edge_set;
    std::copy(edges(g1).first, edges(g1).second, std::back_inserter(edge_set));
    
    std::sort(edge_set.begin(), edge_set.end(), 
              detail::edge_ordering
              (make_iterator_property_map(perm.begin(), index_map1, perm[0]), g1));
    
    std::vector<char> not_in_S_vec(num_vertices(g2), true);
    iterator_property_map<char*, IndexMap2, char, char&>
      not_in_S(&not_in_S_vec[0], index_map2);
    
    return detail::isomorph(g1_vertices.begin(), g1_vertices.end(), 
          edge_set.begin(), edge_set.end(), g1, g2,
          make_iterator_property_map(perm.begin(), index_map1, perm[0]),
          index_map2, f, invar1, invar2, not_in_S);
  }
  

  // Named parameter interface
  template <typename Graph1, typename Graph2, class P, class T, class R>
  bool isomorphism(const Graph1& g1,
                   const Graph2& g2,
                   const bgl_named_params<P,T,R>& params)
  {
    typedef typename graph_traits<Graph2>::vertex_descriptor vertex2_t;
    typename std::vector<vertex2_t>::size_type
      n = is_default_param(get_param(params, vertex_isomorphism_t()))
        ? num_vertices(g1) : 1;
    std::vector<vertex2_t> f(n);
    vertex2_t x;
    degree_vertex_invariant default_invar;
    return isomorphism
      (g1, g2, 
       choose_param
       (get_param(params, vertex_isomorphism_t()),
        make_iterator_property_map
        (f.begin(), 
         choose_const_pmap(get_param(params, vertex_index1),
                     g1, vertex_index), x)),
       choose_param(get_param(params, vertex_invariant_t()),
                    default_invar),
       choose_const_pmap(get_param(params, vertex_index1),
                   g1, vertex_index),
       choose_const_pmap(get_param(params, vertex_index2),
                   g2, vertex_index)
       );
  }

  // All defaults interface
  template <typename Graph1, typename Graph2>
  bool isomorphism(const Graph1& g1, const Graph2& g2)
  {
    typedef typename graph_traits<Graph1>::vertices_size_type size_type;
    typedef typename graph_traits<Graph2>::vertex_descriptor vertex2_t;
    std::vector<vertex2_t> f(num_vertices(g1));
    vertex2_t x;
    degree_vertex_invariant invariant;
    return isomorphism
      (g1, g2,
       make_iterator_property_map(f.begin(), get(vertex_index, g1), x),
       invariant,
       get(vertex_index, g1),
       get(vertex_index, g2)
       );
  }

} // namespace boost

#endif // BOOST_GRAPH_ISOMORPHISM_HPP
