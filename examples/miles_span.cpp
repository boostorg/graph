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

#include <string.h>
#include <stdio.h>
#include <boost/graph/stanford_graph.hpp>
#include <boost/graph/prim_minimum_spanning_tree.hpp>

// A visitor class for accumulating the total length of the minimum
// spanning tree. The Distance template parameter is for a
// PropertyMap.
template <class Distance>
struct total_length_visitor : public boost::ucs_visitor<> {
  typedef typename boost::property_traits<Distance>::value_type D;
  total_length_visitor(D& len, Distance d)
    : _total_length(len), _distance(d) { }
  template <class Vertex, class Graph>
  inline void finish_vertex(Vertex s, Graph& g) {
    _total_length += get(_distance, s); 
  }
  D& _total_length;
  Distance _distance;
};

// We will use the "y" utility field for keeping track of the state
// (color) of each vertex during the algorithm. The algorithm will
// need to access constants for white, gray, and black of the
// appropriate type (the type must match the type of the "y" utility
// field, which in this case is long), hence the following function
// definitions. The choice of constants is arbitrary.
namespace boost {
  long white(long) { return 0; } // "unseen"
  long gray(long) { return 2; }  // "seen"
  long black(long) { return 1; } // "known"
}

int main(int argc, char* argv[])
{
  using namespace boost;
  Graph* g;

  unsigned long n = 100;
  unsigned long n_weight = 0;
  unsigned long w_weight = 0;
  unsigned long p_weight = 0;
  unsigned long d = 10;
  long s = 0;
  unsigned long r = 1;
  char* file_name = NULL;

  while(--argc){
    if(sscanf(argv[argc],"-n%lu",&n)==1);
    else if(sscanf(argv[argc],"-N%lu",&n_weight)==1);
    else if(sscanf(argv[argc],"-W%lu",&w_weight)==1);
    else if(sscanf(argv[argc],"-P%lu",&p_weight)==1);
    else if(sscanf(argv[argc],"-d%lu",&d)==1);
    else if(sscanf(argv[argc],"-r%lu",&r)==1);
    else if(sscanf(argv[argc],"-s%ld",&s)==1);
    else if(strcmp(argv[argc],"-v")==0) verbose = 1;
    else if(strncmp(argv[argc],"-g",2)==0) file_name = argv[argc]+2;
    else{
      fprintf(stderr,
	      "Usage: %s [-nN][-dN][-rN][-sN][-NN][-WN][-PN][-v][-gfoo]\n",
	      argv[0]);
      return -2;
    }
  }
  if (file_name) r = 1;

  while (r--) {
    if (file_name)
      g = restore_graph(file_name);
    else
      g = miles(n,n_weight,w_weight,p_weight,0L,d,s);

    if(g == NULL || g->n <= 1) {
      fprintf(stderr,"Sorry, can't create the graph! (error code %ld)\n",
	      panic_code);
      return-1;
    }

   printf("The graph %s has %ld edges,\n", g->id, g->m / 2);

   long sp_length = 0;

   // Use the "z" utility field for distance.
   typedef property_map<Graph*, z_property<long> >::type Distance;
   Distance d = get(z_property<long>(), g);
   total_length_visitor<Distance> length_vis(sp_length, d);

   prim_minimum_spanning_tree(g, vertex(0,g), 
			      get(z_property<long>(), g),
			      get(edge_length(), g), 
			      // Use the "y" utility field for color.
			      get(y_property<long>(), g),
			      get(vertex_index, g), 
			      length_vis);

   printf("  and its minimum spanning tree has length %ld.\n", sp_length);

   gb_recycle(g);
   s++;
 }
  return 0;
}
