//  (C) Copyright Jeremy Siek 1999. Permission to copy, use, modify,
//  sell and distribute this software is granted provided this
//  copyright notice appears in all copies. This software is provided
//  "as is" without express or implied warranty, and with no claim as
//  to its suitability for any purpose.

#ifndef BOOST_TREE_STRUCTURE_HPP
#define BOOST_TREE_STRUCTURE_HPP

namespace boost {

  template <class T>
  struct tree_traits {
    typedef typename T::node_descriptor node_descriptor;    
    typedef typename T::children_iterator children_iterator;    
  };


  template <class Tree, class TreeVisitor>
  void traverse_tree(typename tree_traits<Tree>::node_descriptor v,
                     Tree& t, TreeVisitor visitor)
  {
    visitor.preorder(v);
    typename tree_traits<Tree>::children_iterator i, end;
    tie(i, end) = children(v, t);
    if (i != end) {
      traverse_tree(*i++, t, visitor);
      visitor.inorder(v);
      while (i != end)
        traverse_tree(*i++, t, visitor);
      visitor.postorder(v);
    } else
      visitor.inorder(v);
  }

  struct null_tree_visitor {
    template <class Node> void preorder(Node) { }
    template <class Node> void inorder(Node) { }
    template <class Node> void postorder(Node) { }
  };

} /* namespace boost */

#endif /* BOOST_TREE_STRUCTURE_HPP */
