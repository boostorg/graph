//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek,
//          Doug Gregor, D. Kevin McGrath
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
#ifndef BOOST_GRAPH_CUTHILL_MCKEE_HPP
#define BOOST_GRAPH_CUTHILL_MCKEE_HPP

#include <boost/config.hpp>
#include <boost/graph/detail/sparse_ordering.hpp>


/*
  (Reverse) Cuthill-McKee Algorithm for matrix reordering
*/

namespace boost {

  namespace detail {



    template < typename OutputIterator, typename Buffer, typename DegreeMap > 
    class bfs_rcm_visitor:public default_bfs_visitor
    {
    public:
      bfs_rcm_visitor(OutputIterator *iter, Buffer *b, DegreeMap deg): 
	permutation(iter), Qptr(b), degree(deg) { }
      template <class Vertex, class Graph>
      void examine_vertex(Vertex u, Graph&) {
	*(*permutation)++ = u;
	index_begin = Qptr->size();
      }
      template <class Vertex, class Graph>
      void finish_vertex(Vertex, Graph&) {

	typedef typename property_traits<DegreeMap>::value_type DS;

	typedef indirect_cmp<DegreeMap, std::less<DS> > Compare;
	Compare comp(degree);
    		
	sort(Qptr->begin()+index_begin, Qptr->end(), comp);
      }
    protected:
      OutputIterator *permutation;
      int index_begin;
      Buffer *Qptr;
      DegreeMap degree;
    };

  } // namespace detail  


  // Reverse Cuthill-McKee algorithm with a given starting Vertex.
  //
  // If user provides a reverse iterator, this will be a reverse-cuthill-mckee
  // algorithm, otherwise it will be a standard CM algorithm

  template <class Graph, class OutputIterator,
            class ColorMap, class DegreeMap>
  OutputIterator
  cuthill_mckee_ordering(Graph& g,
			 std::deque< typename
			 graph_traits<Graph>::vertex_descriptor > vertex_queue,
			 OutputIterator permutation, 
			 ColorMap color, DegreeMap degree)
  {

    //create queue, visitor...don't forget namespaces!
    typedef typename property_traits<DegreeMap>::value_type DS;
    typedef typename graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::sparse::sparse_ordering_queue<Vertex> queue;
    typedef typename detail::bfs_rcm_visitor<OutputIterator, queue, DegreeMap> Visitor;
    typedef typename property_traits<ColorMap>::value_type ColorValue;
    typedef color_traits<ColorValue> Color;


    queue Q;

    //create a bfs_rcm_visitor as defined above
    Visitor	vis(&permutation, &Q, degree);

    typename graph_traits<Graph>::vertex_iterator ui, ui_end;    

    // Copy degree to pseudo_degree
    // initialize the color map
    for (tie(ui, ui_end) = vertices(g); ui != ui_end; ++ui){
      put(color, *ui, Color::white());
    }


    while( !vertex_queue.empty() ) {
      Vertex s = vertex_queue.front();
      vertex_queue.pop_front();
      
      //call BFS with visitor
      breadth_first_visit(g, s, Q, vis, color);
    }
    return permutation;
  }
    

  // This is the case where only a single starting vertex is supplied.
  template <class Graph, class OutputIterator,
            class ColorMap, class DegreeMap>
  OutputIterator
  cuthill_mckee_ordering(Graph& g,
			 typename graph_traits<Graph>::vertex_descriptor s,
			 OutputIterator permutation, 
			 ColorMap color, DegreeMap degree)
  {

    std::deque< typename graph_traits<Graph>::vertex_descriptor > vertex_queue;
    vertex_queue.push_front( s );

    return cuthill_mckee_ordering(g, vertex_queue, permutation, color, degree);
  
  }
  

  // This is the version of CM which selects its own starting vertex
  template < class Graph, class OutputIterator, 
             class Color, class Degree >
  OutputIterator 
  cuthill_mckee_ordering(Graph& G, OutputIterator permutation, 
			     Color color, Degree degree)
  {

    typedef typename boost::graph_traits<Graph>::vertex_descriptor Vertex;
    typedef typename boost::graph_traits<Graph>::vertex_iterator   VerIter;

    VerIter     ri = vertices(G).first;
    Vertex      r = *ri;
    Vertex      s;

    // Check the number of connected components in G.
    std::vector<int>        c( num_vertices( G ) );
    std::deque<Vertex>      vertex_queue;

    int num = boost::connected_components(G, &c[0]);

    // Common case: we only have one set.
    if( num <= 1 ) {
      s = find_starting_node(G, r, color, degree);
      vertex_queue.push_front( s );

    } else {
      // We seem to have more than one disjoint set.  So, find good
      // starting nodes within each of the subgraphs, and then add
      // these starting nodes to our vertex queue.
      int                     num_considered = 0;
      std::vector<int>        sets_considered( num );
      std::fill( sets_considered.begin(), sets_considered.end(), 0 ); // Sanity

      for( unsigned int i = 0; i < c.size(); ++i ) {
        // If it's the first time we've considered this set,
        // then find a good pseudo peripheral node for it.
        // Otherwise, keep going until we've considered all of
        // the sets.
        if( sets_considered[c[i]] == 0 ) {
          ++num_considered;
          s = find_starting_node(G, i, color,
                                 degree);
          assert( c[s] == c[i] );
          vertex_queue.push_back( s );

          if( num_considered >= num ) {
            break;
          }
        }
        ++(sets_considered[c[i]]);
      }
    }
    return cuthill_mckee_ordering(G, vertex_queue, permutation,
			 color, degree);
  }

} // namespace boost


#endif // BOOST_GRAPH_CUTHILL_MCKEE_HPP
