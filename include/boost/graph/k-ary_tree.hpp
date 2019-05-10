//=======================================================================
// Copyright 2018 Jeremy William Murphy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_K_ARY_TREE
#define BOOST_GRAPH_K_ARY_TREE

#if __cplusplus > 201103L

#include <boost/config.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_mutability_traits.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/indexed_properties.hpp>
#include <boost/graph/named_function_params.hpp>

#include <boost/iterator/iterator_facade.hpp>
#include <boost/iterator/transform_iterator.hpp>

#include <boost/concept/assert.hpp>
#include <boost/graph/graph_concepts.hpp>

#include <boost/range.hpp>
#include <boost/range/adaptor/reversed.hpp>

#include <stack>
#include <utility>

namespace boost
{
  enum class visit { pre, in, post };
}

#include <boost/graph/detail/k-ary_tree.hpp>

namespace boost
{
  template <typename Graph>
  bool empty(typename graph_traits<Graph>::vertex_descriptor u, Graph const &)
  {
    return u == graph_traits<Graph>::null_vertex();
  }

  template <bool Predecessor, typename Vertex = std::size_t>
  class binary_tree;

  /******************
   *  Forward tree  *
   ******************/
  template <typename Vertex>
  class binary_tree<false, Vertex>
    : public detail::binary_tree_base<Vertex,
      detail::binary_tree_forward_node<binary_tree<false, Vertex> > >
  {
      typedef detail::binary_tree_base<Vertex,
              detail::binary_tree_forward_node<binary_tree<false, Vertex> > > super_t;

    public:
      using directed_category = directed_tag;
      class traversal_category : public incidence_graph_tag,
        public vertex_list_graph_tag {};
      using edge_descriptor = typename super_t::edge_descriptor;
      using vertex_descriptor = typename super_t::vertex_descriptor;

      using super_t::super_t;

      friend
      edge_descriptor
      add_left_edge(vertex_descriptor parent, vertex_descriptor child,
                         binary_tree &g)
      {
        BOOST_ASSERT(parent != child);

        return g.add_left_edge(parent, child);
      }

      friend
      edge_descriptor
      add_right_edge(vertex_descriptor parent, vertex_descriptor child,
                          binary_tree &g)
      {
        BOOST_ASSERT(parent != child);

        return g.add_right_edge(parent, child);
      }


      // *** MutableGraph interface ***

      friend
      std::pair<edge_descriptor, bool>
      add_edge(vertex_descriptor u, vertex_descriptor v, binary_tree &g)
      {
        return g.add_edge(u, v);
      }

      friend
      void
      remove_edge(vertex_descriptor u, vertex_descriptor v, binary_tree &g)
      {
        g.remove_edge(u, v);
      }

      friend
      void
      remove_edge(edge_descriptor e, binary_tree &g)
      {
        remove_edge(e.first, e.second, g);
      }

      friend
      void
      clear_vertex(vertex_descriptor u, binary_tree &g)
      {
        g.clear_vertex(u);
      }
  };


  /************************
   *  Bidirectional tree  *
   ************************/
  template <typename Vertex>
  class binary_tree<true, Vertex>
    : public detail::binary_tree_base<Vertex,
      detail::binary_tree_bidirectional_node<binary_tree<true, Vertex> > >
  {
      typedef detail::binary_tree_base<Vertex,
              detail::binary_tree_bidirectional_node<binary_tree<true, Vertex> > > super_t;

    public:
      typedef bidirectional_tag directed_category;
      class traversal_category : public bidirectional_graph_tag,
        public vertex_list_graph_tag {};
      typedef typename super_t::edge_descriptor edge_descriptor;
      typedef typename super_t::vertex_descriptor vertex_descriptor;
      typedef typename super_t::degree_size_type degree_size_type;

      using super_t::super_t;

      friend
      edge_descriptor
      add_left_edge(vertex_descriptor parent, vertex_descriptor child,
                         binary_tree &g)
      {
        BOOST_ASSERT(parent != child);

        g.nodes[child].predecessor = parent;
        return g.add_left_edge(parent, child);
      }

      friend
      edge_descriptor
      add_right_edge(vertex_descriptor parent, vertex_descriptor child,
                          binary_tree &g)
      {
        BOOST_ASSERT(parent != child);

        g.nodes[child].predecessor = parent;
        return g.add_right_edge(parent, child);
      }

  public:
      // NOTE: This function will be an infinite loop if called on a recurrent
      // tree (which is not a tree any more).
      friend
      vertex_descriptor
      root(vertex_descriptor u, binary_tree const &g)
      {
        BOOST_ASSERT(!empty(u, g));

        while (has_predecessor(u, g)) {
          u = predecessor(u, g);
        }

        BOOST_ASSERT(u != graph_traits<binary_tree>::null_vertex());
        return u;
      }

      friend
      bool
      is_left_successor(vertex_descriptor u, binary_tree const &g)
      {
        BOOST_ASSERT(!empty(u, g));

        vertex_descriptor v = predecessor(u, g);
        return left_successor(v, g) == u;
      }


      friend
      bool
      is_right_successor(vertex_descriptor u, binary_tree const &g)
      {
        BOOST_ASSERT(!empty(u, g));

        vertex_descriptor v = predecessor(u, g);
        return right_successor(v, g) == u;
      }

      friend
      bool
      has_predecessor(vertex_descriptor u, binary_tree const &g)
      {
        BOOST_ASSERT(!empty(u, g));
        BOOST_ASSERT(u < g.nodes.size());

        return g[u].predecessor != graph_traits<binary_tree>::null_vertex();
      }

      friend
      vertex_descriptor
      predecessor(vertex_descriptor u, binary_tree const &g)
      {
        BOOST_ASSERT(!empty(u, g));
        BOOST_ASSERT(u < g.nodes.size());

        return g[u].predecessor;
      }

      // *** BidirectionalGraph interface ***

    private:
      struct make_in_edge_descriptor {
        make_in_edge_descriptor(vertex_descriptor target) : target(target) {}

        edge_descriptor operator()(vertex_descriptor source) const {
          return edge_descriptor(source, target);
        }
        vertex_descriptor target;
      };

    public:
      typedef transform_iterator<make_in_edge_descriptor, vertex_descriptor const *,
              edge_descriptor> in_edge_iterator;

      friend
      std::pair<in_edge_iterator, in_edge_iterator>
      in_edges(vertex_descriptor u, binary_tree const &g) {
        auto const p = has_predecessor(u, g);
        return std::make_pair(in_edge_iterator(&g.nodes[u].predecessor,
                                               make_in_edge_descriptor(u)),
                              in_edge_iterator(&g.nodes[u].predecessor + p,
                                               make_in_edge_descriptor(u)));
      }

      friend
      degree_size_type
      in_degree(vertex_descriptor u, binary_tree const &g)
      {
        return has_predecessor(u, g);
      }

      friend
      degree_size_type
      degree(vertex_descriptor u, binary_tree const &g)
      {
        return in_degree(u, g) + out_degree(u, g);
      }

      // *** MutableGraph interface ***

      friend
      std::pair<edge_descriptor, bool>
      add_edge(vertex_descriptor u, vertex_descriptor v, binary_tree &g)
      {
        if (!g.add_vertex(v) && predecessor(v, g) != g.null_vertex())
          return std::make_pair(edge_descriptor(), false);
        g.add_vertex(u);

        std::pair<edge_descriptor, bool> const result = g.add_edge_strict(u, v);
        if (result.second)
          g.nodes[v].predecessor = u;
        return result;
      }

      friend
      void
      remove_edge(vertex_descriptor u, vertex_descriptor v, binary_tree &g)
      {
        BOOST_ASSERT(predecessor(v, g) == u);
        g.remove_edge(u, v);
        g.nodes[v].predecessor = super_t::null_vertex();
      }

      friend
      void
      remove_edge(edge_descriptor e, binary_tree &g)
      {
        remove_edge(e.first, e.second, g);
      }

      friend
      void
      clear_vertex(vertex_descriptor u, binary_tree &g)
      {
        g.clear_childrens_predecessor(u);
        g.clear_vertex(u);
        g.nodes[u].predecessor = super_t::null_vertex();
      }


      friend
      void
      remove_vertex(vertex_descriptor u, binary_tree &g)
      {
        BOOST_ASSERT(in_degree(u, g) == 0);

        g.remove_vertex(u);
      }


      void
      clear_childrens_predecessor(vertex_descriptor u)
      {
        for (int i = 0; i != 2; i++) {
          super_t::nodes[super_t::nodes[u].successors[i]].predecessor = super_t::null_vertex();
        }
      }
  };

  // IncidenceGraph interface

  template <typename Vertex = std::size_t>
  using forward_binary_tree = binary_tree<false, Vertex>;
  template <typename Vertex = std::size_t>
  using bidirectional_binary_tree = binary_tree<true, Vertex>;

  namespace detail
  {
    template <typename BinaryTree, typename Visitor>
    Visitor traverse_nonempty(vertex_descriptor_t<BinaryTree> u,
                              BinaryTree const &g, Visitor vis)
    {
      vis(visit::pre, u);
      if (has_left_successor(u, g))
        vis = traverse_nonempty(left_successor(u, g), g, vis);
      vis(visit::in, u);
      if (has_right_successor(u, g))
        vis = traverse_nonempty(right_successor(u, g), g, vis);
      vis(visit::post, u);
      return vis;
    }


    template <typename BinaryTree>
    int traverse_step(visit &v, vertex_descriptor_t<BinaryTree> &u,
                      BinaryTree const &g)
    {
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree>));

      switch (v) {
      case visit::pre:
        if (has_left_successor(u, g)) {
          u = left_successor(u, g);                   return 1;
        } v = visit::in;                              return 0;
      case visit::in:
        if (has_right_successor(u, g)) {
          v = visit::pre; u = right_successor(u, g);  return 1;
        } v = visit::post;                            return 0;
      case visit::post:
        if (is_left_successor(u, g)) {
          v = visit::in;
        }                 u = predecessor(u, g);      return -1;
      }
    }

    template <typename BinaryTree, typename Visitor>
    Visitor traverse(vertex_descriptor_t<BinaryTree> u,
                     BinaryTree const &g, Visitor vis)
    {
      if (empty(u, g))
        return vis;
      auto root = u;
      visit v = visit::pre;
      vis(v, u);
      do {
        traverse_step(v, u, g);
        vis(v, u);
      } while (u != root || v != visit::post);
      return vis;
    }


    template <typename BinaryTree0, typename BinaryTree1>
    bool bifurcate_isomorphic_nonempty(
      vertex_descriptor_t<BinaryTree0> u, BinaryTree0 const &g,
      vertex_descriptor_t<BinaryTree1> v, BinaryTree1 const &h)
    {
      BOOST_CONCEPT_ASSERT((concepts::ForwardBinaryTreeConcept<BinaryTree0>));
      BOOST_CONCEPT_ASSERT((concepts::ForwardBinaryTreeConcept<BinaryTree1>));
      BOOST_ASSERT(!empty(u, g));
      BOOST_ASSERT(!empty(v, h));

      if (has_left_successor(u, g)) {
        if (has_left_successor(v, h)) {
          if (!bifurcate_isomorphic_nonempty(left_successor(u, g), g,
                                             left_successor(v, h), h))
            return false;
        } else
          return false;
      } else if (has_left_successor(u, g))
        return false;

      if (has_right_successor(u, g)) {
        if (has_right_successor(v, h)) {
          if (!bifurcate_isomorphic_nonempty(right_successor(u, g), g,
                                             right_successor(v, h), h))
            return false;
        } else
          return false;
      } else if (has_right_successor(u, g))
        return false;

      return true;
    }


    template <typename BinaryTree0, typename BinaryTree1>
    bool bifurcate_isomorphic(
                      vertex_descriptor_t<BinaryTree0> u, BinaryTree0 const &g,
                      vertex_descriptor_t<BinaryTree1> v, BinaryTree1 const &h)
    {
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree0>));
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree1>));

      if (empty(u, g)) return empty(v, h);
      if (empty(v, h)) return false;
      auto root0 = u;
      visit visit0 = visit::pre;
      visit visit1 = visit::pre;
      while (true) {
        traverse_step(visit0, u, g);
        traverse_step(visit1, v, h);
        if (visit0 != visit1) return false;
        if (u == root0 && visit0 == visit::post) return true;
      }
    }
  }


  template <typename Vertex, typename DFSTreeVisitor>
  void
  depth_first_search(binary_tree<false, Vertex> &g,
                     vertex_descriptor_t<binary_tree<false, Vertex>> s,
                     DFSTreeVisitor &vis)
  {
    if (!empty(s, g))
      vis = detail::traverse_nonempty(s, g, vis);
  }


  template <typename Vertex, typename DFSTreeVisitor>
  void
  depth_first_search(binary_tree<true, Vertex> &g,
                     vertex_descriptor_t<binary_tree<true, Vertex>> s,
                     DFSTreeVisitor &vis)
  {
    vis = detail::traverse(s, g, vis);
  }


  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(binary_tree<false, Vertex0> const &g,
              binary_tree<false, Vertex1> const &h)
  {
    if (num_vertices(g) != num_vertices(h))
      return false;
    return num_vertices(g) == 0
           || detail::bifurcate_isomorphic_nonempty(0, g, 0, h);
  }


  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(binary_tree<true, Vertex0> const &g,
              binary_tree<true, Vertex1> const &h)
  {
    if (num_vertices(g) != num_vertices(h))
      return false;
    return num_vertices(g) == 0 ||
           detail::bifurcate_isomorphic(0, g, 0, h);
  }
}

#endif // #if __cplusplus > 201103L

#endif // #ifndef BOOST_GRAPH_K_ARY_TREE
