/* Copyright 2002 Björn Lindberg
   Permission to copy, use, modify, sell and distribute this software
   is granted provided this copyright notice appears in all
   copies. This software is provided "as is" without express or
   implied warranty, and with no claim as to its suitability for any
   purpose.
 */

// Revision History
// July 7, 2002    Initial checkin -Jeremy Siek

#ifndef BOOST_TRAVERSE_TREE_GRAPH_HPP
#define BOOST_TRAVERSE_TREE_GRAPH_HPP
 
#include <boost/graph/graph_traits.hpp>
 
namespace boost {
  
    template<typename TreeGraph, typename TreeGraphVisitor>
    void traverse_tree_graph(
	typename boost::graph_traits<TreeGraph>::vertex_descriptor v,
	TreeGraph & g,
	TreeGraphVisitor visitor)
    {
	visitor.preorder(v, g);
	typename boost::graph_traits<TreeGraph>::adjacency_iterator i, end;
	tie(i, end) = adjacent_vertices(v, g);
	if (i != end) { // if current node is not a leaf
	    visitor.on_edge(boost::edge(v, *i, g).first, g);
	    traverse_tree_graph(*i, g, visitor);
	    visitor.postedge(boost::edge(v, *i++, g).first, g);
	    while (i != end) {
		visitor.inorder(v, g);
		visitor.on_edge(boost::edge(v, *i, g).first, g);
		traverse_tree_graph(*i, g, visitor);
		visitor.postedge(boost::edge(v, *i++, g).first, g);
	    }
	} else
	    visitor.inorder(v, g);
	visitor.postorder(v, g);
  }
    
} // namespace boost
 
#endif // BOOST_TRAVERSE_TREE_GRAPH_HPP
