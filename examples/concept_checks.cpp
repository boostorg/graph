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
#include <boost/graph/graph_concepts.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/edge_list.hpp>
#include <boost/graph/stanford_graph.hpp>

//
// These concept checks verify that the GGCL classes implement the
// complete interfaces defined in the graph concepts. The concept
// checks are also useful for verifying whether user-defined graph
// classes satisfy the desired graph concepts. Note that these concept
// checks by themselves do not verify whether the classes model the
// concepts, for there are run-time requirements that are not checked
// here.
//

int
main(int,char*[])
{
  using namespace boost;
  {
    typedef adjacency_list<vecS, vecS, directedS, 
     plugin<color_tag,int>, plugin<weight_tag,int> > Graph;
    REQUIRE(Graph, VertexAndEdgeListGraph);
    REQUIRE(Graph, MutableGraph);
    REQUIRE2(Graph, color_tag, VertexPropertyGraph);
    REQUIRE2(Graph, weight_tag, EdgePropertyGraph);
    // the builtin id property is readable but not writable
    typedef vertex_property_accessor<Graph,id_tag>::const_type ID_PA;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    REQUIRE2(ID_PA, Vertex, ReadablePropertyAccessor);
  }
  {
    typedef adjacency_list<vecS, vecS, bidirectionalS, plugin<color_tag,int>,
      plugin<weight_tag,int> > Graph;
    REQUIRE(Graph, VertexAndEdgeListGraph);
    REQUIRE(Graph, BidirectionalGraph);
    REQUIRE(Graph, MutableGraph);
    REQUIRE2(Graph, color_tag, VertexPropertyGraph);
    REQUIRE2(Graph, weight_tag, EdgePropertyGraph);
    // the builtin id property is readable but not writable
    typedef vertex_property_accessor<Graph,id_tag>::const_type ID_PA;
    typedef boost::graph_traits<Graph>::vertex_descriptor Vertex;
    REQUIRE2(ID_PA, Vertex, ReadablePropertyAccessor);
  }
  {
    typedef adjacency_list< listS, listS, directedS, plugin<color_tag,int>, 
      plugin<weight_tag,int> > Graph;
    REQUIRE(Graph, VertexAndEdgeListGraph);
    REQUIRE(Graph, MutableGraph);
    REQUIRE2(Graph, color_tag, VertexPropertyGraph);
    REQUIRE2(Graph, weight_tag, EdgePropertyGraph);
  }
  {
    typedef std::pair<int,int> E;
    typedef edge_list<E*,E,ptrdiff_t> EdgeList;
    REQUIRE(EdgeList, EdgeListGraph);
  }
  { // Stanford GraphBase Graph
    typedef Graph* Graph;
    REQUIRE(Graph, VertexListGraph);
  }
  return 0;
}
