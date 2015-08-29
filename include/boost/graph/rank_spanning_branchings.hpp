//=======================================================================
// Copyright 2015 Clemson University
// Authors: Bradley S. Meyer
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
#ifndef BOOST_GRAPH_RANK_SPANNING_BRANCHINGS_HPP
#define BOOST_GRAPH_RANK_SPANNING_BRANCHINGS_HPP

/*
 * Rank spanning branchings
 *         Camerini et al Algorithm
 *
 * Requirement:
 *      directed graph with single root vertex
 */

#include <vector>
#include <limits>

#include <boost/property_map/property_map.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/unordered_set.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

namespace boost {

  typedef adjacency_list < vecS, vecS, bidirectionalS,
    no_property, no_property > BranchingGraph;

  typedef graph_traits<BranchingGraph>::vertex_descriptor BranchingVertex;

  namespace detail {

    template <class Edge, class WeightMap, class Compare>
    struct EdgeNode
    {
      Edge                                                            edge;
      typename property_traits<WeightMap>::value_type                 weight;
      Compare                                                         compare;
      EdgeNode(){}
      EdgeNode(
        const Edge& e,
        const typename property_traits<WeightMap>::value_type & w,
        Compare c ) :
        edge( e ), weight( w ), compare( c ) {}
      bool operator<( EdgeNode const & rhs ) const
      { return compare( weight, rhs.weight ); }
    };

    // Insert edges from graph into heap, taking into account constraints.

    template <class Graph, class Vertex, class Edge, class WeightMap,
              class Compare>
    bool
    insert_edges
    (
      const Graph& g,
      WeightMap& w,
      Compare& comp,
      std::vector<heap::fibonacci_heap<EdgeNode<Edge,WeightMap,Compare> > >&
        in_edges,
      const unordered_set<Edge>& include_edges,
      const unordered_set<Edge>& exclude_edges
    )
    {
      unordered_set<Vertex> include_vertices, in_set, out_set;
      typename unordered_set<Vertex>::iterator it;

      // Insert vertices in sets to check for spanning branching.

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        in_set.insert( v );
        out_set.insert( v );
      }

      // Insert edges that must be present and note the invertex.  Remove
      // vertices from relevant sets.

      BOOST_FOREACH( const Edge& e, include_edges )
      {
        include_vertices.insert( target( e, g ) );
        in_edges[target( e, g )].push(
          EdgeNode<Edge,WeightMap,Compare>( e, get( w, e ), comp )
        );
        it = in_set.find( target( e, g ) );
        if( it != in_set.end() ) { in_set.erase( it ); }
        it = out_set.find( source( e, g ) );
        if( it != out_set.end() ) { out_set.erase( it ); }
      }
        
      // Insert edges, but not edges to be excluded or into vertices that
      // already have in edges.  Remove vertices from relevant sets.

      BOOST_FOREACH( const Edge &e, edges(g) )
      {
        if(
          include_vertices.find( target(e, g) ) == include_vertices.end()
          &&
          exclude_edges.find( e ) == exclude_edges.end()
        )
        {
          if( source(e, g) != target(e, g) )
          {
            in_edges[target(e,g)].push(
              EdgeNode<Edge,WeightMap,Compare>( e, get( w, e ), comp )
            );
            it = in_set.find( target( e, g ) );
            if( it != in_set.end() ) { in_set.erase( it ); }
            it = out_set.find( source( e, g ) );
            if( it != out_set.end() ) { out_set.erase( it ); }
          }
        }
      }

      // Check for correct number of roots.
      //
      if( in_set.size() != 1 )
        return false;   // Zero roots or more than one root.
      else if( out_set.find( *in_set.begin() ) != out_set.end() )
        return false;   // Root is isolated.
      else
        return true;

    }

    // Retrieve the path back from leaf to root in cycle branching.

    template <class BranchingGraph, class BranchingVertex>
    void
    find_back_path(
      BranchingGraph& C,
      std::vector<BranchingVertex>& bv
    )
    {

      BGL_FORALL_INEDGES_T( bv[0], e, C, BranchingGraph )
      {
        bv.insert( bv.begin(), source( e, C ) );
        find_back_path( C, bv );
      }

    }

    // Expand cycles.

    template <class Graph, class Edge, class BranchingGraph,
              class BranchingVertex, class WeightMap, class Compare>
    void
    expand(
      const Graph& g,
      BranchingGraph& C,
      unordered_set<BranchingVertex>& root_set,
      std::vector<EdgeNode< Edge, WeightMap, Compare> >& beta
    )
    {

      BOOST_FOREACH( BranchingVertex v, root_set )
      {
        if( in_degree( v, C ) == 0 && out_degree( v, C ) != 0 ) 
        {
          std::vector<BranchingVertex> bv;
          bv.push_back( target( beta[v].edge, g ) );
          find_back_path( C, bv );
          for( size_t i = 0; i < bv.size() - 1; i++ )
          {
            beta[bv[i+1]] = beta[bv[i]];
            clear_vertex( bv[i], C );
          }
        }
      }

      // Remove isolated vertices.

      std::vector<BranchingVertex> vertices_to_remove_from_set;

      BOOST_FOREACH( BranchingVertex v, root_set )
      {
        if( in_degree( v, C ) == 0 && out_degree( v, C ) == 0 ) 
        {
          vertices_to_remove_from_set.push_back( v );
        }
      }

      BOOST_FOREACH( BranchingVertex v, vertices_to_remove_from_set )
      {
        root_set.erase( v );
      }

    }

    // Camerini et al. BEST routine.

    template <class Graph, class Edge, class WeightMap, class Compare>
    void
    best_spanning_branching( const Graph& g, 
                             unordered_set<Edge>& branching,
                             WeightMap& w,
                             Compare& comp,
                             unordered_set<Edge>& include_edges,
                             unordered_set<Edge>& exclude_edges
                           )
    {
      if (num_vertices(g) == 0) return; // Nothing to do in this case

      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::vertices_size_type vertex_idx_t;

      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<WeightMap, Edge> ));

      typedef EdgeNode<Edge, WeightMap, Compare> edge_node_t;

      typedef heap::fibonacci_heap<edge_node_t> f_heap_t;

      unordered_set<Vertex> unvisited_vertex_set;

      Vertex root_vertex;

      BranchingGraph C;

      vertex_idx_t n = num_vertices(g);

      disjoint_sets_with_storage<> W( n ), S( n );

      std::vector<f_heap_t> in_edges( n );

      if(
        !insert_edges<Graph, Vertex, Edge, WeightMap, Compare>
        (
          g,
          w,
          comp,
          in_edges,
          include_edges,
          exclude_edges
        )
      ) return;

      std::vector<edge_node_t> beta( 2 * n );

      std::vector<Vertex> parent( n );

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        W.make_set( v );
        S.make_set( v );
        parent[v]  = v;
        add_vertex( C );
        unvisited_vertex_set.insert( v );
      }

      while( !unvisited_vertex_set.empty() )
      {

        typename unordered_set<Vertex>::iterator it =
          unvisited_vertex_set.begin(); 

        if( in_edges[*it].empty() )
        {
          root_vertex = *it;
	  unvisited_vertex_set.erase( it );
        }
        else
        {

	  edge_node_t critical_edge_node = in_edges[*it].top();

	  beta[parent[*it]] = critical_edge_node;

	  unvisited_vertex_set.erase( it );

	  if(
	    W.find_set( source( critical_edge_node.edge, g ) ) !=
	    W.find_set( target( critical_edge_node.edge, g ) )
	  )
	  {
	    W.union_set(
              source( critical_edge_node.edge, g ),
              target( critical_edge_node.edge, g )
	    );
	  }
	  else
	  {
	    Vertex v_new = add_vertex( C );

	    edge_node_t least_costly_edge_node = critical_edge_node; 

	    boost::unordered_set<Vertex> cycle_vertex_set;

	    for(
              Vertex v = source( critical_edge_node.edge, g );
	      cycle_vertex_set.find( S.find_set( v ) ) ==
                cycle_vertex_set.end();
	      v = source( beta[parent[S.find_set( v )]].edge, g )
	    )
	    {
              Vertex u = S.find_set( v );
	      cycle_vertex_set.insert( u );
	      add_edge( v_new, parent[u], C ); 
              if(
                comp( beta[parent[u]].weight, least_costly_edge_node.weight )
              )
              {
		least_costly_edge_node = beta[parent[u]];
              }
	    }

            Vertex new_repr = *cycle_vertex_set.begin();

	    BOOST_FOREACH( Vertex u, cycle_vertex_set )
	    {
              S.link( u, new_repr );
              new_repr = S.find_set( new_repr );
            }

            std::vector<shared_ptr<edge_node_t> > v_exit( n );
	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit )
              {
                t.reset();
              }
              BOOST_FOREACH( edge_node_t en, in_edges[v] )
              {
                if( S.find_set( source( en.edge, g ) ) !=
                    new_repr
                )
                {
                  en.weight += least_costly_edge_node.weight -
                      beta[parent[v]].weight;
                  Vertex u = S.find_set( source( en.edge, g ) );
                  if( v_exit[u] )
                  {
                    if( comp( (*v_exit[u]).weight, en.weight ) )
                    {
                      v_exit[u].reset( new edge_node_t );
                      *v_exit[u] = en;
                    }
                  }
                  else
                  {
                    v_exit[u].reset( new edge_node_t );
                    *v_exit[u] = en;
                  }
                }
	      }
              f_heap_t tmp_heap;
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit )
              {
                if( t ) {tmp_heap.push( *t );}
              }
              in_edges[v].swap( tmp_heap );
	    }
                 
	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              if( v != new_repr ) in_edges[new_repr].merge( in_edges[v] );
            }

	    unvisited_vertex_set.insert( new_repr );

            parent[new_repr] = v_new;

	  }

        }

      }

      // Create a set containing possible roots of the cycle branching.
      // In each cycle expansion, remove isolated vertices of C to avoid
      // considering them in subsequent cycle expansions.

      unordered_set<BranchingVertex> root_set;

      BGL_FORALL_VERTICES( v, C, BranchingGraph )
      {
        root_set.insert( v );
      }

      while( !root_set.empty() )
      {
        expand( g, C, root_set, beta );
      }

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        if( v != root_vertex )
        {
          branching.insert( beta[v].edge );
        }
      }

    }

    // Depth-first visitor to set up pre-order and post-order maps.

    template<typename OrderMap>
    class dfs_order_visitor:public default_dfs_visitor
    {

      public:
	dfs_order_visitor(
	  OrderMap& pr,
	  OrderMap& po
	) :  m_pr( pr ), m_po( po ) { td = 0; tf = 0; }

	template<typename Vertex, typename Graph>
	void discover_vertex(Vertex u, const Graph & g)
	{
	  m_pr[u] = td++;
	}

	template < typename Vertex, typename Graph >
	void finish_vertex(Vertex u, const Graph & g)
	{
	  m_po[u] = tf++;
	}

	OrderMap& m_pr;
	OrderMap& m_po;

      private:
	size_t td;
	size_t tf;

    };

    // The ancestor checker.  A vertex u is a proper ancestor of
    // vertex v (in the parent branching) if pr[u] < pr[v] and po[u] > po[v].

    template<typename OrderMap>
    struct ancestor_checker
    {
      OrderMap& m_pr;
      OrderMap& m_po;
      ancestor_checker( OrderMap& pr, OrderMap& po ) :
	m_pr( pr ), m_po( po ) {}

      template<typename Vertex>
      bool operator()( const Vertex& v1, const Vertex& v2 ) const
      { return ( m_pr[v1] < m_pr[v2] && m_po[v1] > m_po[v2] ); }
    };

    // Camerini et al. NEXT routine.

    template <class Graph, class Edge, class WeightMap, class Compare>
    shared_ptr<
      std::pair<Edge, typename property_traits<WeightMap>::value_type>
    >
    next_spanning_branching( const Graph& g, 
                             const unordered_set<Edge>& branching,
                             WeightMap& w,
                             Compare& comp,
                             unordered_set<Edge>& include_edges,
                             unordered_set<Edge>& exclude_edges
                           )
    {

      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::vertices_size_type vertex_idx_t;

      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<WeightMap, Edge> ));

      Edge return_edge;

      typedef typename property_traits<WeightMap>::value_type weight_t;

      scoped_ptr<weight_t> delta;

      typedef EdgeNode<Edge, WeightMap, Compare> edge_node_t;

      edge_node_t b;

      // Initialize delta. 

      delta.reset();

      // Create a branching graph to check whether a vertex is an ancestor of
      // another vertex in the branching.

      vertex_idx_t n = num_vertices(g);

      BranchingGraph C;

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        add_vertex( C );
      }

      BOOST_FOREACH( const Edge& e, branching )
      {
        add_edge( source( e, g), target( e, g ), C );
      }

      // Create other types and data.

      typedef heap::fibonacci_heap<edge_node_t> f_heap_t;

      shared_ptr<
        std::pair<Edge, typename property_traits<WeightMap>::value_type>
      > p(
          new std::pair<Edge, typename property_traits<WeightMap>::value_type>
        );

      unordered_set<Vertex> unvisited_vertex_set;

      disjoint_sets_with_storage<> W( n ), S( n );

      std::vector<f_heap_t> in_edges( n );

      std::vector<edge_node_t> max_e( n );

      if(
        !insert_edges<Graph, Vertex, Edge, WeightMap, Compare>
        (
          g,
          w,
          comp,
          in_edges,
          include_edges,
          exclude_edges
        )
      )
      {
        p.reset();
        return p;
      }

      // Initialize data structures and find start vertex for depth
      // first search.

      Vertex start_vertex;

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        W.make_set( v );
        S.make_set( v );
        unvisited_vertex_set.insert( v );
        if( in_edges[v].empty() ) { start_vertex = v; }
      }

      // Create the ancestor checker from C.

      typedef std::map<Vertex, size_t> OrderMap;

      OrderMap pr;
      OrderMap po;

      dfs_order_visitor<OrderMap> vis(pr, po);

      depth_first_search(C, visitor(vis).root_vertex( start_vertex ) );

      ancestor_checker<OrderMap> is_ancestor(pr, po);

      // Main loop.

      while( !unvisited_vertex_set.empty() )
      {

        typename unordered_set<Vertex>::iterator it =
          unvisited_vertex_set.begin(); 

        if( in_edges[*it].empty() )
        {
	  unvisited_vertex_set.erase( it );
        }
        else
        {

          // Get largest in edge with ties solved in favor of edges in
          // input branching.

          for(
            typename f_heap_t::ordered_iterator ei =
              in_edges[*it].ordered_begin();
            ei != in_edges[*it].ordered_end();
            ei++
          )
          {
            if( comp( (*ei).weight, in_edges[*it].top().weight ) )
              break;
            b = *ei;
            if( branching.find( b.edge ) != branching.end() )
              break;
          }

	  max_e[*it] = b;

          // Camerini et al. SEEK routine.
                  
          if( branching.find( b.edge ) != branching.end() )
          {
            for(
              typename f_heap_t::ordered_iterator ei =
                in_edges[*it].ordered_begin();
              ei != in_edges[*it].ordered_end();
              ei++
            )
            {
              if( (*ei).edge != b.edge )
              {
                if(
                  !is_ancestor(
                    target( b.edge, g ),
                    source( (*ei).edge, g )
                  )
                )
                {
                  if( !delta )
                  {
                    delta.reset( new weight_t );
                    *delta = b.weight - (*ei).weight;
                    return_edge = b.edge;
                    break;
                  }
                  else if(
                    comp( b.weight - (*ei).weight, *delta )
                  )
                  {
                    *delta = b.weight - (*ei).weight;
                    return_edge = b.edge;
                    break;
                  }
                }
              }
            }
          }

	  unvisited_vertex_set.erase( it );

	  if(
	    W.find_set( source( b.edge, g ) ) !=
	    W.find_set( target( b.edge, g ) )
	  )
	  {
	    W.union_set( source( b.edge, g ), target( b.edge, g ) );
	  }
	  else
	  {

	    edge_node_t least_costly_edge_node = b; 

	    boost::unordered_set<Vertex> cycle_vertex_set;

	    for(
              Vertex v = source( b.edge, g );
	      cycle_vertex_set.find( S.find_set( v ) ) ==
                cycle_vertex_set.end();
	      v = source( max_e[S.find_set( v )].edge, g )
	    )
	    {
              Vertex u = S.find_set( v );
	      cycle_vertex_set.insert( u );
	      if( comp( max_e[u].weight, least_costly_edge_node.weight ) )
              {
		least_costly_edge_node = max_e[u];
              }
	    }

            Vertex new_repr = *cycle_vertex_set.begin();

	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              S.link( v, new_repr );
              new_repr = S.find_set( new_repr );
            }

            // Adjust arc weights and remove parallel arcs.  Keep the
            // the largest weight in arc and the largest viable alternative
            // arc from each source outside the cycle.

            std::vector<shared_ptr<edge_node_t> > v_exit1( n ), v_exit2( n );
	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit1 )
              {
                t.reset();
              }
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit2 )
              {
                t.reset();
              }
              BOOST_FOREACH( edge_node_t en, in_edges[v] )
              {
                if( S.find_set( source( en.edge, g ) ) != new_repr )
                {
                  en.weight += least_costly_edge_node.weight - max_e[v].weight;
                  Vertex u = S.find_set( source( en.edge, g ) );
                  if( v_exit1[u] )
                  {
                    if( comp( (*v_exit1[u]).weight, en.weight ) )
                    {
                      if(
                        !is_ancestor(
                           target( (*v_exit1[u]).edge, g ),
                           source( (*v_exit1[u]).edge, g )
                        )
                      )
                      {
                        v_exit2[u].reset( new edge_node_t );
                        *v_exit2[u] = *v_exit1[u];
                      }
                      *v_exit1[u] = en;
                    }
                    else if(
                      !v_exit2[u] || comp( (*v_exit2[u]).weight, en.weight )
                    )
                    {
                      if(
                        !is_ancestor(
                            target( en.edge, g ), source( en.edge, g )
                        )
                      )
                      {
                        v_exit2[u].reset( new edge_node_t );
                        *v_exit2[u] = en;
                      }
                    }
                  }
                  else
                  {
                    v_exit1[u].reset( new edge_node_t );
                    *v_exit1[u] = en;
                  }
                }
	      }
              f_heap_t tmp_heap;
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit1 )
              {
                if( t ) {tmp_heap.push( *t );}
              }
              BOOST_FOREACH( shared_ptr<edge_node_t> & t, v_exit2 )
              {
                if( t ) {tmp_heap.push( *t );}
              }
              in_edges[v].swap( tmp_heap );
	    }
                 
	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              if( v != new_repr ) in_edges[new_repr].merge( in_edges[v] );
            }

	    unvisited_vertex_set.insert( new_repr );

	  }

        }

      }

      if( delta )
      {
        *p = std::make_pair( return_edge, *delta );
        return p;
      }
      else
      {
        p.reset();
        return p;
      }

    }

  } // namespace detail 

  template<class Edge, class WeightMap, class Compare>
  struct BranchingEntry
  {
    Edge                                                        edge;
    typename property_traits<WeightMap>::value_type             weight;
    Compare                                                     compare;
    unordered_set<Edge>                                         branching;
    unordered_set<Edge>                                         include_edges;
    unordered_set<Edge>                                         exclude_edges;
    BranchingEntry(
      const typename property_traits<WeightMap>::value_type& w,
      const Edge& e,
      Compare& comp,
      const unordered_set<Edge>& b,
      const unordered_set<Edge>& include,
      const unordered_set<Edge>& exclude
    ) :
        edge( e ), weight( w ), compare( comp ),
        branching( b ), include_edges( include ),
        exclude_edges( exclude ){}
    bool operator<( BranchingEntry const & rhs ) const
    { return compare( weight, rhs.weight ); }
  };

  template<class WeightMap, class Edge>
  typename property_traits<WeightMap>::value_type
  compute_branching_weight(
    WeightMap& w,
    const unordered_set<Edge>& branching
  )
  {

    typename property_traits<WeightMap>::value_type weight = 0;

     BOOST_FOREACH( const Edge& e, branching )
     {
       weight += get( w, e );
     }

     return weight;
  }
   
  template <class Graph, class Function, class Compare>
  inline void 
  rank_spanning_branchings( const Graph& g,
                            size_t k,
                            Function fn,
                            Compare comp
                          )
  {

    typedef typename graph_traits<Graph>::edge_descriptor Edge;

    BOOST_CONCEPT_ASSERT(( VertexListGraphConcept<Graph> ));
    BOOST_CONCEPT_ASSERT(( EdgeListGraphConcept<Graph> ));

    unordered_set<Edge> best_branching, empty_set;

    typedef typename property_map<Graph, edge_weight_t>::const_type WeightMap;

    WeightMap w = get( edge_weight, g );

    typedef typename property_traits<WeightMap>::value_type weight_t;

    shared_ptr< std::pair<Edge, weight_t> > p;

    Edge e;

    heap::fibonacci_heap<BranchingEntry<Edge, WeightMap, Compare> > Q;

    detail::best_spanning_branching( g,
                                     best_branching,
                                     w,
                                     comp,
                                     empty_set,
                                     empty_set
                                   );

    if( !fn( g, best_branching ) || k == 1 ) return;

    p = detail::next_spanning_branching( g,
                                         best_branching,
                                         w,
                                         comp,
                                         empty_set,
                                         empty_set
                                       );

    if( p )
    {
      Q.push(
        BranchingEntry<Edge, WeightMap, Compare>(
          compute_branching_weight( w, best_branching ) - (*p.get()).second,
          (*p.get()).first,
          comp,
          best_branching,
          empty_set,
          empty_set
        )
      );
    }
    else
    {
      return;
    }

    for( size_t j = 1; j < k; j++ )
    {

      if( Q.empty() ) break;

      unordered_set<Edge> branching;

      BranchingEntry<Edge, WeightMap, Compare> P = Q.top();

      Q.pop();

      unordered_set<Edge> include_edges = P.include_edges;

      unordered_set<Edge> exclude_edges = P.exclude_edges;

      include_edges.insert( P.edge );

      exclude_edges.insert( P.edge );

      detail::best_spanning_branching( g,
                                       branching,
                                       w,
                                       comp,
                                       P.include_edges,
                                       exclude_edges
                                     );

      if( !fn( g, branching ) || j == k - 1 ) return;

      p =
        detail::next_spanning_branching( g,
                                         P.branching,
                                         w,
                                         comp,
                                         include_edges,
                                         P.exclude_edges
                                       );

      if( p )
      {
        Q.push(
          BranchingEntry<Edge, WeightMap, Compare>(
            compute_branching_weight( w, P.branching ) - (*p.get()).second,
            (*p.get()).first,
            comp,
            P.branching,
            include_edges,
            P.exclude_edges
          )
        );
      }

      p =
        detail::next_spanning_branching( g,
                                         branching,
                                         w,
                                         comp,
                                         P.include_edges,
                                         exclude_edges
                                       );

      if( p )
      {
        Q.push(
          BranchingEntry<Edge, WeightMap, Compare>(
            P.weight - (*p.get()).second,
            (*p.get()).first,
            comp,
            branching,
            P.include_edges,
            exclude_edges
          )
        );
      }

    }

  }

  template <class Graph, class Function>
  inline void 
  rank_spanning_branchings( const Graph& g,
                            size_t k,
                            Function fn
                          )
  {

    typedef
      typename
      property_traits<
        typename property_map<Graph, edge_weight_t>::const_type
      >::value_type weight_t;

    rank_spanning_branchings( g, k, fn, std::less<weight_t>() );

  }

} // namespace boost

#endif // BOOST_GRAPH_RANK_SPANNING_BRANCHINGS_HPP
