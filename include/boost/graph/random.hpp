//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
//

// Copyright 2004 The Trustees of Indiana University.

// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  Authors: Douglas Gregor
//           Andrew Lumsdaine

// Copyright (C) Vladimir Prus 2003. Permission to copy, use,
// modify, sell and distribute this software is granted provided this
// copyright notice appears in all copies. This software is provided
// "as is" without express or implied warranty, and with no claim as
// to its suitability for any purpose.

#ifndef BOOST_GRAPH_RANDOM_HPP
#define BOOST_GRAPH_RANDOM_HPP

#include <boost/graph/graph_traits.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/random/uniform_01.hpp>
#include <boost/random/linear_congruential.hpp>

#include <boost/pending/property.hpp>
#include <boost/graph/properties.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/copy.hpp>
#include <boost/mpl/if.hpp>
#include <boost/type_traits/is_convertible.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/shared_ptr.hpp>

#include <iostream>

namespace boost {

  // grab a random vertex from the graph's vertex set
  template <class Graph, class RandomNumGen>
  typename graph_traits<Graph>::vertex_descriptor
  random_vertex(Graph& g, RandomNumGen& gen)
  {
    if (num_vertices(g) > 1) {
      uniform_int<> distrib(0, num_vertices(g)-1);
      variate_generator<RandomNumGen&, uniform_int<> > rand_gen(gen, distrib);
      std::size_t n = rand_gen();
      typename graph_traits<Graph>::vertex_iterator
        i = vertices(g).first;
      while (n-- > 0) ++i; // std::advance not VC++ portable
      return *i;
    } else
      return *vertices(g).first;
  }

  template <class Graph, class RandomNumGen>
  typename graph_traits<Graph>::edge_descriptor
  random_edge(Graph& g, RandomNumGen& gen) {
    if (num_edges(g) > 1) {
      uniform_int<> distrib(0, num_edges(g)-1);
      variate_generator<RandomNumGen&, uniform_int<> > rand_gen(gen, distrib);
      typename graph_traits<Graph>::edges_size_type 
        n = rand_gen();
      typename graph_traits<Graph>::edge_iterator
        i = edges(g).first;
      while (n-- > 0) ++i; // std::advance not VC++ portable
      return *i;
    } else
      return *edges(g).first;
  }

  namespace detail {
    class dummy_property_copier {
    public:
      template<class V1, class V2>
      void operator()(const V1&, const V2&) const {}
    };
  }

  template <typename MutableGraph, class RandNumGen>
  void generate_random_graph1
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     bool allow_parallel = true,
     bool self_edges = false)
  {
    typedef graph_traits<MutableGraph> Traits;
    typedef typename Traits::vertices_size_type v_size_t;
    typedef typename Traits::edges_size_type e_size_t;
    typedef typename Traits::vertex_descriptor vertex_descriptor;

    // When parallel edges are not allowed, we create a new graph which
    // does not allow parallel edges, construct it and copy back.
    // This is not efficient if 'g' already disallow parallel edges,
    // but that's task for later.
    if (!allow_parallel) {

      typedef typename boost::graph_traits<MutableGraph>::directed_category dir;      
      typedef typename mpl::if_<is_convertible<dir, directed_tag>,
          directedS, undirectedS>::type select;
      adjacency_list<setS, vecS, select> g2;
      generate_random_graph1(g2, V, E, gen, true, self_edges);

      copy_graph(g2, g, vertex_copy(detail::dummy_property_copier()).
                        edge_copy(detail::dummy_property_copier()));

    } else {

      for (v_size_t i = 0; i < V; ++i)
        add_vertex(g);
      
      for (e_size_t j = 0; j < E; ++j) {
        vertex_descriptor a = random_vertex(g, gen), b;
        do {
          b = random_vertex(g, gen);
        } while (self_edges == false && a == b);
        add_edge(a, b, g);
      }
    }
  }

  template <typename MutableGraph, class RandNumGen>
  void generate_random_graph
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     bool allow_parallel = true,
     bool self_edges = false)
  {
      generate_random_graph1(g, V, E, gen, allow_parallel, self_edges);
  }

  template <typename MutableGraph, typename RandNumGen,
            typename VertexOutputIterator, typename EdgeOutputIterator>
  void generate_random_graph
    (MutableGraph& g, 
     typename graph_traits<MutableGraph>::vertices_size_type V,
     typename graph_traits<MutableGraph>::vertices_size_type E,
     RandNumGen& gen,
     VertexOutputIterator vertex_out,
     EdgeOutputIterator edge_out,
     bool self_edges = false)
  {
    typedef graph_traits<MutableGraph> Traits;
    typedef typename Traits::vertices_size_type v_size_t;
    typedef typename Traits::edges_size_type e_size_t;
    typedef typename Traits::vertex_descriptor vertex_t;
    typedef typename Traits::edge_descriptor edge_t;

    for (v_size_t i = 0; i < V; ++i)
      *vertex_out++ = add_vertex(g);

    for (e_size_t j = 0; j < E; ++j) {
      vertex_t a = random_vertex(g, gen), b;
      do {
        b = random_vertex(g, gen);
      } while (self_edges == false && a == b);
      edge_t e; bool inserted;
      tie(e, inserted) = add_edge(a, b, g);
      if (inserted)
        *edge_out++ = std::make_pair(source(e, g), target(e, g));
    }
  }

  namespace detail {

    template<class Property, class G, class RandomGenerator>
    void randomize_property(G& g, RandomGenerator& rg, 
                            Property, vertex_property_tag)
    {
      typename property_map<G, Property>::type pm = get(Property(), g);
      typename graph_traits<G>::vertex_iterator vi, ve;
      for (tie(vi, ve) = vertices(g); vi != ve; ++vi) {
        pm[*vi] = rg();
      }
    }

    template<class Property, class G, class RandomGenerator>
    void randomize_property(G& g, RandomGenerator& rg, 
                            Property, edge_property_tag)
    {
      typename property_map<G, Property>::type pm = get(Property(), g);
      typename graph_traits<G>::edge_iterator ei, ee;
      for (tie(ei, ee) = edges(g); ei != ee; ++ei) {
        pm[*ei] = rg();
      }
    }
  }

  template<class Property, class G, class RandomGenerator>
  void randomize_property(G& g, RandomGenerator& rg)
  {
    detail::randomize_property
        (g, rg, Property(), typename property_kind<Property>::type());
  }

  template<typename SizeType, typename Probability, 
           typename RandomGenerator = minstd_rand>
  class erdos_renyi_random_iterator
    : public iterator_facade<
             erdos_renyi_random_iterator<SizeType,Probability,RandomGenerator>
             , std::pair<SizeType, SizeType> 
             , std::input_iterator_tag
             , std::pair<SizeType, SizeType> 
             >
  {
    typedef erdos_renyi_random_iterator self_type;
    typedef iterator_facade<
               erdos_renyi_random_iterator<SizeType,Probability,RandomGenerator>
             , std::pair<SizeType, SizeType> 
             , std::input_iterator_tag
             , std::pair<SizeType, SizeType> 
             > inherited;
  public:
    typedef typename inherited::reference reference;
    typedef typename inherited::value_type value_type;
    
    // Starting iterator for n vertices with probability p using gen
    // as the random number generator.
    erdos_renyi_random_iterator(SizeType n, Probability p, 
                                RandomGenerator& gen, bool directed,
                                bool self_edges = false)
      : n(n), p(p), 
        random_(new uniform_01<RandomGenerator, Probability>(gen)), 
        directed(directed), self_edges(self_edges),
        current_edge(static_cast<SizeType>(0), static_cast<SizeType>(0))
    {
      for (/* no init */; current_edge.first < n; ++current_edge.first)
      {
        if (directed) current_edge.second = static_cast<SizeType>(0);
        else current_edge.second = current_edge.first;
        for (/* no init */; current_edge.second < n; ++current_edge.second) {
          // When we hit the first edge, break out
          if ((self_edges || current_edge.first != current_edge.second)
              && (*random_)() < p) {
            return;
          }
        }
      }
    }

    // Past-the-end iterator
    erdos_renyi_random_iterator(SizeType n) : n(n), current_edge(n, n) { }

  private:
    reference dereference() const { return current_edge; }
    
    bool equal(const self_type& other) const 
    { return current_edge == other.current_edge; }

    void increment()
    {
      do {
        if (++current_edge.second == n) {
          if (++current_edge.first == n) break;
          if (directed) current_edge.second = static_cast<SizeType>(0);
          else current_edge.second = current_edge.first;
        }
      } while ((!self_edges && current_edge.first == current_edge.second)
               || (*random_)() >= p);
    }
                                
    SizeType    n;
    Probability p;
    shared_ptr<uniform_01<RandomGenerator, Probability> > random_;
    bool        directed : 1;
    bool        self_edges : 1;
    value_type  current_edge;
    friend class iterator_core_access;
  };

  template<typename SizeType, typename Probability, typename RandomGenerator>
  erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator>
  make_erdos_renyi_random_iterator(SizeType n, Probability p, 
                                   RandomGenerator& gen,
                                   directed_tag,
                                   bool self_edges = false)
  {
    typedef erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator> 
      result_type;
    return result_type(n, p, gen, true, self_edges);
  }

  template<typename SizeType, typename Probability, typename RandomGenerator>
  erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator>
  make_erdos_renyi_random_iterator(SizeType n, Probability p, 
                                   RandomGenerator& gen,
                                   undirected_tag,
                                   bool self_edges = false)
  {
    typedef erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator> 
      result_type;
    return result_type(n, p, gen, false, self_edges);
  }

  template<typename SizeType, typename Probability, typename RandomGenerator>
  erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator>
  make_erdos_renyi_random_iterator(SizeType n, Probability, RandomGenerator&)
  {
    typedef erdos_renyi_random_iterator<SizeType, Probability, RandomGenerator> 
      result_type;
    return result_type(n);
  }
}


#endif
