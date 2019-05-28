///=======================================================================
// Copyright 2015-2018 Clemson University
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

#include <boost/concept_check.hpp>
#include <boost/config.hpp>
#include <boost/foreach.hpp>
#include <boost/graph/properties.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/named_function_params.hpp>
#include <boost/graph/iteration_macros.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/heap/fibonacci_heap.hpp>
#include <boost/pending/disjoint_sets.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/optional.hpp>
#include <boost/unordered_set.hpp>

namespace boost {

  namespace detail {

    typedef adjacency_list < vecS, vecS, bidirectionalS,
      no_property, no_property > BranchingGraph;

    typedef graph_traits<BranchingGraph>::vertex_descriptor BranchingVertex;

    // Structure to store edges and compare them by weight.

    template <typename Edge, typename WeightMap, typename Compare>
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

    // Alias template for heap used in the routine.  Current version
    // is the boost::fibonacci_heap.  The heap must be mergable.

    template<typename T>
    struct MyHeap
    { 
      typedef heap::fibonacci_heap<T> type;
    };

    // Insert edges from graph into heap, taking into account constraints.

    template <typename Graph, typename Vertex, typename Edge,
              typename WeightMap, typename Compare>
    bool
    insert_edges
    (
      const Graph& g,
      WeightMap& weight_map,
      Compare& comp,
      std::map<
        Vertex,
        typename MyHeap<EdgeNode<Edge,WeightMap,Compare> >::type
      >& in_edges,
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
          EdgeNode<Edge,WeightMap,Compare>( e, get( weight_map, e ), comp )
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
              EdgeNode<Edge,WeightMap,Compare>( e, get( weight_map, e ), comp )
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

    void
    find_back_path(
      BranchingGraph& cycle_branching,
      std::vector<BranchingVertex>& bv
    )
    {

      BGL_FORALL_INEDGES( bv[0], e, cycle_branching, BranchingGraph )
      {
        bv.insert( bv.begin(), source( e, cycle_branching ) );
        find_back_path( cycle_branching, bv );
      }

    }

    // Expand cycles.

    template <typename Graph, typename Edge, typename IndexMap,
              typename WeightMap, typename Compare>
    void
    expand(
      const Graph& g,
      IndexMap& v_id,
      BranchingGraph& cycle_branching,
      unordered_set<BranchingVertex>& root_set,
      std::vector<EdgeNode< Edge, WeightMap, Compare> >& beta
    )
    {

      BOOST_FOREACH( BranchingVertex v, root_set )
      {
        if(
          in_degree( v, cycle_branching ) == 0 &&
          out_degree( v, cycle_branching ) != 0
        )
        {
          std::vector<BranchingVertex> bv;
          bv.push_back( v_id[target( beta[v].edge, g )] );
          find_back_path( cycle_branching, bv );
          for( std::size_t i = 0; i < bv.size() - 1; i++ )
          {
            beta[bv[i+1]] = beta[bv[i]];
            clear_vertex( bv[i], cycle_branching );
          }
        }
      }

      // Remove isolated vertices.

      std::vector<BranchingVertex> vertices_to_remove_from_set;

      BOOST_FOREACH( BranchingVertex v, root_set )
      {
        if(
           in_degree( v, cycle_branching ) == 0 &&
           out_degree( v, cycle_branching ) == 0
        )
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

    template <typename Graph, typename Edge, typename IndexMap,
              typename WeightMap, typename Rank, typename Pred,
              typename Compare>
    void
    best_spanning_branching( const Graph& g, 
                             unordered_set<Edge>& branching,
                             IndexMap& v_id,
                             WeightMap& weight_map,
                             Compare& comp,
                             Rank rank,
                             Pred pred1,
                             Pred pred2,
                             unordered_set<Edge>& include_edges,
                             unordered_set<Edge>& exclude_edges
                           )
    {

      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::vertices_size_type vertex_idx_t;

      typedef EdgeNode<Edge, WeightMap, Compare> edge_node_t;

      typedef typename MyHeap<edge_node_t>::type heap_t;
      typedef std::map<Vertex, edge_node_t > exit_map_t;

      unordered_set<Vertex> unvisited_vertex_set;

      Vertex root_vertex;

      BranchingGraph cycle_branching;

      vertex_idx_t n = num_vertices(g);

      disjoint_sets<Rank, Pred> W( rank, pred1 ), S( rank, pred2 );

      std::map<Vertex, heap_t> in_edges;

      if(
        !insert_edges<Graph, Vertex, Edge, WeightMap, Compare>
        (
          g,
          weight_map,
          comp,
          in_edges,
          include_edges,
          exclude_edges
        )
      ) return;

      std::vector<edge_node_t> beta( 2 * n );

      std::map<Vertex, vertex_idx_t> parent;

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        W.make_set( v );
        S.make_set( v );
        parent[v]  = v_id[v];
        add_vertex( cycle_branching );
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
	    BranchingVertex v_new = add_vertex( cycle_branching );

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
	      add_edge( v_new, parent[u], cycle_branching ); 
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

	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              exit_map_t v_exit;
              BOOST_FOREACH( edge_node_t en, in_edges[v] )
              {
                if( S.find_set( source( en.edge, g ) ) != new_repr )
                {
                  en.weight += least_costly_edge_node.weight -
                      beta[parent[v]].weight;
                  Vertex u = S.find_set( source( en.edge, g ) );
                  if( v_exit.find(u) != v_exit.end() )
                  {
                    if( comp( v_exit[u].weight, en.weight ) )
                    {
                      v_exit[u] = en;
                    }
                  }
                  else
                  {
                    v_exit[u] = en;
                  }
                }
	      }
              heap_t tmp_heap;
              BOOST_FOREACH( typename exit_map_t::value_type& t, v_exit )
              {
                tmp_heap.push( t.second );
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
      // In each cycle expansion, remove isolated vertices of the
      // cycle branching to avoid considering them in subsequent cycle
      // expansions.

      unordered_set<BranchingVertex> root_set;

      BGL_FORALL_VERTICES( u, cycle_branching, BranchingGraph )
      {
        root_set.insert( u );
      }

      while( !root_set.empty() )
      {
        expand( g, v_id, cycle_branching, root_set, beta );
      }

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        if( v != root_vertex )
        {
          branching.insert( beta[v_id[v]].edge );
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
	void discover_vertex(const Vertex u, const Graph & g)
	{
	  m_pr[u] = td++;
	}

	template <typename Vertex, typename Graph>
	void finish_vertex(const Vertex u, const Graph & g)
	{
	  m_po[u] = tf++;
	}

	OrderMap& m_pr;
	OrderMap& m_po;

      private:
	std::size_t td;
	std::size_t tf;

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

    template <typename Graph, typename Edge, typename IndexMap,
              typename WeightMap, typename Rank, typename Pred,
              typename Compare>
    boost::optional<
      std::pair<Edge, typename property_traits<WeightMap>::value_type>
    >
    next_spanning_branching( const Graph& g, 
                             const unordered_set<Edge>& branching,
                             IndexMap& v_id,
                             WeightMap& weight_map,
                             Compare& comp,
                             Rank rank,
                             Pred pred1,
                             Pred pred2,
                             unordered_set<Edge>& include_edges,
                             unordered_set<Edge>& exclude_edges
                           )
    {

      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

      Edge return_edge;

      typedef typename property_traits<WeightMap>::value_type weight_t;

      boost::optional<weight_t> delta;

      typedef EdgeNode<Edge, WeightMap, Compare> edge_node_t;

      edge_node_t b;

      // Create a branching graph to check whether a vertex is an ancestor of
      // another vertex in the branching.

      BranchingGraph ancestor_branching;

      for( size_t i = 0; i < num_vertices( g ); i++ )
      {
        add_vertex( ancestor_branching );
      }

      BOOST_FOREACH( const Edge& e, branching )
      {
        add_edge(
          v_id[source( e, g )], v_id[target( e, g )], ancestor_branching
        );
      }

      // Create other types and data.

      typedef typename MyHeap<edge_node_t>::type heap_t;
      typedef std::map<Vertex, edge_node_t > exit_map_t;

      boost::optional<
        std::pair<Edge, typename property_traits<WeightMap>::value_type>
      > next_edge_and_weight_delta;

      unordered_set<Vertex> unvisited_vertex_set;

      disjoint_sets<Rank, Pred> W( rank, pred1 ), S( rank, pred2 );

      std::map<Vertex, heap_t> in_edges;

      std::map<Vertex, edge_node_t> max_e;

      if(
        !insert_edges<Graph, Vertex, Edge, WeightMap, Compare>
        (
          g,
          weight_map,
          comp,
          in_edges,
          include_edges,
          exclude_edges
        )
      )
      {
        return next_edge_and_weight_delta;
      }

      // Initialize data structures and find start vertex for depth
      // first search.

      BranchingVertex start_vertex;

      BGL_FORALL_VERTICES_T( v, g, Graph )
      {
        W.make_set( v );
        S.make_set( v );
        unvisited_vertex_set.insert( v );
        if( in_edges[v].empty() ) { start_vertex = v_id[v]; }
      }

      // Create the ancestor checker from ancestor_branching.

      typedef std::map<BranchingVertex, std::size_t> OrderMap;

      OrderMap pr;
      OrderMap po;

      dfs_order_visitor<OrderMap> vis(pr, po);

      depth_first_search(
        ancestor_branching, visitor(vis).root_vertex( start_vertex )
      );

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
            typename heap_t::ordered_iterator ei =
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
              typename heap_t::ordered_iterator ei =
                in_edges[*it].ordered_begin();
              ei != in_edges[*it].ordered_end();
              ei++
            )
            {
              if( (*ei).edge != b.edge )
              {
                if(
                  !is_ancestor(
                    v_id[target( b.edge, g )],
                    v_id[source( (*ei).edge, g )]
                  )
                )
                {
                  if( !delta )
                  {
                    delta = b.weight - (*ei).weight;
                    return_edge = b.edge;
                    break;
                  }
                  else if(
                    comp( b.weight - (*ei).weight, delta.get() )
                  )
                  {
                    delta = b.weight - (*ei).weight;
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
            // arc from each source outside the cycle.  Make sure that
            // an arc from the branching is among the added edges, if present.

	    BOOST_FOREACH( Vertex v, cycle_vertex_set )
	    {
              exit_map_t b_exit, v_exit1, v_exit2;
              BOOST_FOREACH( edge_node_t en, in_edges[v] )
              {
                if( S.find_set( source( en.edge, g ) ) != new_repr )
                {
                  en.weight += least_costly_edge_node.weight - max_e[v].weight;
                  Vertex u = S.find_set( source( en.edge, g ) );
                  if( branching.find( en.edge ) != branching.end() )
                  {
                    b_exit[u] = en;
                  }
                  else
                  {
                    if( v_exit1.find(u) != v_exit1.end() )
                    {
                      if( comp( v_exit1[u].weight, en.weight ) )
                      {
                        if(
                          !is_ancestor(
                             v_id[target( v_exit1[u].edge, g )],
                             v_id[source( v_exit1[u].edge, g )]
                          )
                        )
                        {
                          v_exit2[u] = v_exit1[u];
                        }
                        v_exit1[u] = en;
                      }
                      else if(
                        v_exit2.find(u) == v_exit2.end() ||
                        comp( v_exit2[u].weight, en.weight )
                      )
                      {
                        if(
                          !is_ancestor(
                            v_id[target( en.edge, g )],
                            v_id[source( en.edge, g )]
                          )
                        )
                        {
                          v_exit2[u] = en;
                        }
                      }
                    }
                    else
                    {
                      v_exit1[u] = en;
                    }
                  }
                }
	      }
              heap_t tmp_heap;
              BOOST_FOREACH( typename exit_map_t::value_type& t, b_exit )
              {
                tmp_heap.push( t.second );
              }
              BOOST_FOREACH( typename exit_map_t::value_type& t, v_exit1 )
              {
                tmp_heap.push( t.second );
              }
              BOOST_FOREACH( typename exit_map_t::value_type& t, v_exit2 )
              {
                tmp_heap.push( t.second );
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
        next_edge_and_weight_delta = std::make_pair( return_edge, delta.get() );
      }

      return next_edge_and_weight_delta;

    }

    // Class to filter graph.

    template<typename EdgeSet>
    class branching_filter
    {

      public:
        branching_filter(){}
        branching_filter( const EdgeSet * _es ) : p_es( _es ){}

        template <typename Edge>
        bool operator()( const Edge& e ) const
        {
          if( p_es->find( e ) != p_es->end() )
            return true;
          else
            return false;
        }

     private:
       const EdgeSet * p_es;

    };

    // Structure to store branchings and compare them by weight.

    template<typename Edge, typename WeightMap, typename Compare>
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

    // Compute the weight of a branching.

    template<typename WeightMap, typename Edge>
    typename property_traits<WeightMap>::value_type
    compute_branching_weight(
      WeightMap& w,
      const unordered_set<Edge>& branching
    )
    {

       typedef typename property_traits<WeightMap>::value_type weight_t;

       boost::optional<weight_t> weight;

       BOOST_FOREACH( const Edge& e, branching )
       {
         
         if( !weight )
         {
           weight = get( w, e );
         }
         else
         {
	   weight = weight.get() + get( w, e );
         }
       }

       return weight.get();

    }

    // Routine implementation.
     
    template <typename Graph, typename BranchingProcessor, typename IndexMap,
              typename WeightMap, typename Compare, typename Rank,
              typename Parent>
    void 
    rank_spanning_branchings_impl( const Graph& g,
			           BranchingProcessor bp,
                                   IndexMap v_id,
                                   WeightMap w,
			           Compare comp,
                                   Rank rank,
                                   Parent pred1,
                                   Parent pred2
			         )
    {

      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
      typedef typename graph_traits<Graph>::edge_descriptor Edge;

      typedef typename property_traits<WeightMap>::value_type weight_t;

      BOOST_CONCEPT_ASSERT(( VertexAndEdgeListGraphConcept<Graph> ));

      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<IndexMap, Vertex> ));
      BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<WeightMap, Edge> ));

      unordered_set<Edge> best_branching, empty_set;

      boost::optional<std::pair<Edge, weight_t> > next_edge_and_weight_delta;

      Edge e;

      typename MyHeap<BranchingEntry<Edge, WeightMap, Compare> >::type
        branching_heap;

      best_spanning_branching( g,
                               best_branching,
                               v_id,
                               w,
                               comp,
                               rank,
                               pred1,
                               pred2,
                               empty_set,
                               empty_set
                             );

      branching_filter<unordered_set<Edge> > filter1( &best_branching );
      filtered_graph<Graph, branching_filter<unordered_set<Edge> > > fg1( g, filter1 );

      if( !bp( fg1 ) ) return;

      next_edge_and_weight_delta =
        next_spanning_branching( g,
                                 best_branching,
                                  v_id,
                                  w,
                                  comp,
                                  rank,
                                  pred1,
                                  pred2,
                                  empty_set,
                                  empty_set
                               );

      if( next_edge_and_weight_delta )
      {
	branching_heap.push(
	  BranchingEntry<Edge, WeightMap, Compare>(
	    compute_branching_weight( w, best_branching ) -
              (next_edge_and_weight_delta.get()).second,
	    (next_edge_and_weight_delta.get()).first,
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

      while( !branching_heap.empty() )
      {

	unordered_set<Edge> branching;

	BranchingEntry<Edge, WeightMap, Compare> P = branching_heap.top();

	branching_heap.pop();

	unordered_set<Edge> include_edges = P.include_edges;

	unordered_set<Edge> exclude_edges = P.exclude_edges;

	include_edges.insert( P.edge );

	exclude_edges.insert( P.edge );

	best_spanning_branching( g,
                                 branching,
                                 v_id,
                                 w,
                                 comp,
                                 rank,
                                 pred1,
                                 pred2,
                                 P.include_edges,
                                 exclude_edges
                               );

        branching_filter<unordered_set<Edge> > filter( &branching );
        filtered_graph<Graph, branching_filter<unordered_set<Edge > > > fg( g, filter );

        if( !bp( fg ) ) return;

	next_edge_and_weight_delta =
	  next_spanning_branching( g,
                                   P.branching,
                                   v_id,
                                   w,
                                   comp,
                                   rank,
                                   pred1,
                                   pred2,
                                   include_edges,
                                   P.exclude_edges
                                 );

	if( next_edge_and_weight_delta )
	{
	  branching_heap.push(
	    BranchingEntry<Edge, WeightMap, Compare>(
	      compute_branching_weight( w, P.branching ) -
                (next_edge_and_weight_delta.get()).second,
	      (next_edge_and_weight_delta.get()).first,
	      comp,
	      P.branching,
	      include_edges,
	      P.exclude_edges
	    )
	  );
	}

	next_edge_and_weight_delta =
	  next_spanning_branching( g,
                                   branching,
                                   v_id,
                                   w,
                                   comp,
                                   rank,
                                   pred1,
                                   pred2,
                                   P.include_edges,
                                   exclude_edges
                                 );

	if( next_edge_and_weight_delta )
	{
	  branching_heap.push(
	    BranchingEntry<Edge, WeightMap, Compare>(
	      P.weight - (next_edge_and_weight_delta.get()).second,
	      (next_edge_and_weight_delta.get()).first,
	      comp,
	      branching,
	      P.include_edges,
	      exclude_edges
	    )
	  );
	}

      }

    }

    template <typename Graph, typename BranchingProcessor, typename IndexMap,
              typename WeightMap, typename Compare>
    void 
    rank_spanning_branchings_dispatch2( const Graph& g,
                                        BranchingProcessor bp,
                                        IndexMap id_map,
                                        WeightMap weight_map,
                                        Compare compare
                                      )
    {

      typename graph_traits<Graph>::vertices_size_type n = num_vertices(g);

      if( num_vertices( g ) == 0 ) return; // Nothing to do.

      typedef typename graph_traits<Graph>::vertices_size_type vertex_idx_t;
      typedef typename graph_traits<Graph>::vertex_descriptor Vertex;

      // Set up rank and parent for disjoint sets.
      
      std::vector<vertex_idx_t> rank( n );

      std::vector<Vertex> pred1( n ), pred2( n );

      rank_spanning_branchings_impl(
        g,
        bp,
        id_map,
        weight_map,
        compare,
        make_iterator_property_map( rank.begin(), id_map, rank[0] ),
        make_iterator_property_map( pred1.begin(), id_map, pred1[0] ),
        make_iterator_property_map( pred2.begin(), id_map, pred2[0])
      );

    }

    template <typename Graph, typename BranchingProcessor, typename Compare,
              typename P, typename T, typename R>
    void rank_spanning_branchings_dispatch1(
      const Graph& g,
      BranchingProcessor bp,
      Compare compare,
      const bgl_named_params<P, T, R>& params
    )
    {

      detail::rank_spanning_branchings_dispatch2(
        g,
        bp,
        choose_param(
          get_param( params, vertex_index_t()), get( vertex_index, g )
        ),
        choose_param(
          get_param( params, edge_weight_t()), get( edge_weight, g )
        ),
        compare
      );

    }
 
    template <typename Graph, typename BranchingProcessor, typename P,
              typename T, typename R>
    void rank_spanning_branchings_dispatch1(
      const Graph& g,
      BranchingProcessor bp,
      param_not_found,
      const bgl_named_params<P, T, R>& params
    )
    {

      typedef
        typename
        property_traits<
          typename property_map<Graph, edge_weight_t>::const_type
        >::value_type weight_t;
 
      BOOST_CONCEPT_ASSERT(( ComparableConcept<weight_t> ));

      detail::rank_spanning_branchings_dispatch2(
        g,
        bp,
        choose_param(
          get_param( params, vertex_index_t()), get( vertex_index, g )
        ),
        choose_param(
          get_param( params, edge_weight_t()), get( edge_weight, g )
        ),
        std::less<weight_t>()
      );

    }
 
  } // namespace detail 

  template <typename Graph, typename BranchingProcessor, typename P, typename T,
            typename R>
  inline void 
  rank_spanning_branchings(
    const Graph& g,
    BranchingProcessor bp,
    const bgl_named_params<P, T, R>& params
  )
  {

    detail::rank_spanning_branchings_dispatch1(
      g,
      bp,
      get_param( params, distance_compare_t() ),
      params
    );
 
  }

  template <typename Graph, typename Function>
  inline void 
  rank_spanning_branchings( const Graph& g,
                            Function bp
                          )
  {

    bgl_named_params<int,int> params(0);
    rank_spanning_branchings( g, bp, params );

  }

} // namespace boost

#endif // BOOST_GRAPH_RANK_SPANNING_BRANCHINGS_HPP
