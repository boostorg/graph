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
/*

  Paul Moore's request:

  As an example of a practical problem which is not restricted to graph
  "experts", consider file dependencies. It's basically graph construction,
  plus topological sort, but it might make a nice "tutorial" example. Build a
  dependency graph of files, then use the algorithms to do things like
  
  1. Produce a full recompilation order (topological sort, by modified date)
  2. Produce a "parallel" recompilation order (same as above, but group files
  which can be built in parallel)
  3. Change analysis (if I change file x, which others need recompiling)
  4. Dependency changes (if I add a dependency between file x and file y, what
  are the effects)
  
*/

#include <iostream>
#include <iterator>
#include <algorithm>
#include <time.h>

#include <boost/utility.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/topological_sort.hpp>
#include <boost/graph/depth_first_search.hpp>
#include <boost/graph/uniform_cost_search.hpp>
#include <boost/graph/visitors.hpp>

using namespace std;
using namespace boost;

enum files_e { dax_h, yow_h, boz_h, zow_h, foo_cpp, 
               foo_o, bar_cpp, bar_o, libfoobar_a,
               zig_cpp, zig_o, zag_cpp, zag_o, 
                 libzigzag_a, killerapp, N };
const char* name[] = { "dax.h", "yow.h", "boz.h", "zow.h", "foo.cpp",
                       "foo.o", "bar.cpp", "bar.o", "libfoobar.a",
                       "zig.cpp", "zig.o", "zag.cpp", "zag.o",
                       "libzigzag.a", "killerapp" };


struct print_visitor : public bfs_visitor<> {
  template <class Vertex, class Graph>
  void discover_vertex(Vertex v, Graph&) {
    cout << name[v] << " ";
  }
};


template <class Color>
struct cycle_detector : public dfs_visitor<>
{
  typedef typename boost::property_traits<Color>::value_type C;

  cycle_detector(Color c, bool& has_cycle) 
    : _has_cycle(has_cycle), color(c) { }

  template <class Edge, class Graph>
  void back_edge(Edge e, Graph& g) {
    _has_cycle = true;
  }
protected:
  bool& _has_cycle;
  Color color;
};

int main(int,char*[])
{
    
  typedef pair<int,int> Edge;
  Edge used_by[] = {
    Edge(dax_h, foo_cpp), Edge(dax_h, bar_cpp), Edge(dax_h, yow_h),
    Edge(yow_h, bar_cpp), Edge(yow_h, zag_cpp),
    Edge(boz_h, bar_cpp), Edge(boz_h, zig_cpp), Edge(boz_h, zag_cpp),
    Edge(zow_h, foo_cpp), 
    Edge(foo_cpp, foo_o),
    Edge(foo_o, libfoobar_a),
    Edge(bar_cpp, bar_o),
    Edge(bar_o, libfoobar_a),
    Edge(libfoobar_a, libzigzag_a),
    Edge(zig_cpp, zig_o),
    Edge(zig_o, libzigzag_a),
    Edge(zag_cpp, zag_o),
    Edge(zag_o, libzigzag_a),
    Edge(libzigzag_a, killerapp)
  };
  const int nedges = sizeof(used_by)/sizeof(Edge);
  int weights[nedges];
  fill(weights, weights + nedges, 1);

  typedef adjacency_list<vecS, vecS, directedS, 
      plugin<color_tag, default_color_type>,
      plugin<weight_tag, int>
    > Graph;
  Graph g(N, used_by, used_by + nedges, weights);
  typedef graph_traits<Graph>::vertex_descriptor Vertex;

  typedef vertex_property_accessor<Graph, color_tag>::type Color;
  Color color = get_vertex_property_accessor(g, color_tag());

  // Determine ordering for a full recompilation
  {
    typedef list<Vertex> MakeOrder;
    MakeOrder make_order;
    topological_sort(g, std::front_inserter(make_order));
    
    cout << "make ordering: ";
    for (MakeOrder::iterator i = make_order.begin();
         i != make_order.end(); ++i)
      cout << name[*i] << " ";
    cout << endl;
  }
  cout << endl;

  // Recompilation order with files that can be compiled in parallel
  // grouped together
  {
    // Set up the necessary graph properties.
    vector<int> time(N, 0);
    typedef vector<int>::iterator Time;
    typedef edge_property_accessor<Graph, weight_tag>::type Weight;
    Weight weight = get_edge_property_accessor(g, weight_tag());

    // Calculate the in_degree for each vertex.
    vector<int> in_degree(N, 0);
    graph_traits<Graph>::vertex_iterator i, iend;
    graph_traits<Graph>::out_edge_iterator j, jend;
    for (tie(i, iend) = vertices(g); i != iend; ++i)
      for (tie(j, jend) = out_edges(*i,g); j != jend; ++j)
        in_degree[target(*j,g)] += 1;

    std::greater<int> compare;
    std::plus<int> combine;

    // Run best-first-search from each vertex with zero in-degree.
    for (tie(i, iend) = vertices(g); i != iend; ++i) {
      if (in_degree[*i] == 0)
        uniform_cost_search(g, *i, time.begin(), weight, 
			    compare, combine);
    }

    cout << "parallel make ordering, " << endl
         << "vertices with same group number can be made in parallel" << endl;
    for (tie(i,iend) = vertices(g); i != iend; ++i)
      cout << "time_slot[" << name[*i] << "] = " << time[*i] << endl;
  }
  cout << endl;

  // if I change yow.h what files need to be re-made?
  {
    cout << "A change to yow.h will cause what to be re-made?" << endl;
    print_visitor vis;
    breadth_first_search(g, vertex(yow_h, g), vis);
    cout << endl;
  }
  cout << endl;

  // are there any cycles in the graph?
  {
    bool has_cycle = false;
    cycle_detector<Color> vis(color, has_cycle);
    depth_first_search(g, vis);
    cout << "The graph has a cycle? " << has_cycle << endl;
  }
  cout << endl;

  // add a dependency going from bar.cpp to dax.h
  {
    cout << "adding edge bar_cpp -> dax_h" << endl;
    add_edge(g, bar_cpp, dax_h);
  }
  cout << endl;

  // are there any cycles in the graph?
  {
    typedef vertex_property_accessor<Graph,color_tag>::type Color;
    Color color = get_vertex_property_accessor(g, color_tag());
    bool has_cycle = false;
    cycle_detector<Color> vis(color, has_cycle);
    depth_first_search(g, vis);
    cout << "The graph has a cycle now? " << has_cycle << endl;
  }

  return 0;
}
