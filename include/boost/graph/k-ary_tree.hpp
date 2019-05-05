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

#include <utility>

namespace boost
{
  enum class visit { pre, in, post };
}

#include <boost/graph/detail/k-ary_tree.hpp>

namespace boost
{
  template <typename Graph>
  bool empty(typename graph_traits<Graph>::vertex_descriptor u, Graph const&)
  {
    return u == graph_traits<Graph>::null_vertex();
  }

  template <std::size_t K, bool Predecessor, typename Vertex = std::size_t>
  class k_ary_tree;

  /******************
   *  Forward tree  *
   ******************/
  template <std::size_t K, typename Vertex>
  class k_ary_tree<K, false, Vertex>
    : public detail::k_ary_tree_base<K, Vertex,
              detail::k_ary_tree_forward_node<k_ary_tree<K, false, Vertex> > >
  {
    typedef detail::k_ary_tree_base<K, Vertex,
      detail::k_ary_tree_forward_node<k_ary_tree<K, false, Vertex> > > super_t;

  public:
    using directed_category = directed_tag;
    class traversal_category : public incidence_graph_tag,
                               public vertex_list_graph_tag {};
    using edge_descriptor = typename super_t::edge_descriptor;
    using vertex_descriptor = typename super_t::vertex_descriptor;

    using super_t::super_t;

    friend
    void add_left_edge(vertex_descriptor parent, vertex_descriptor child,
                       k_ary_tree &g)
    {
      BOOST_ASSERT(parent != child);

      g.add_left_edge(parent, child);
    }

    friend
    void add_right_edge(vertex_descriptor parent, vertex_descriptor child,
                        k_ary_tree &g)
    {
      BOOST_ASSERT(parent != child);

      g.add_right_edge(parent, child);
    }


    // *** VertexListGraph interface ***

    struct vertex_iterator : public iterator_facade<
                                                  vertex_iterator,
                                                  vertex_descriptor const,
                                                  multi_pass_input_iterator_tag
                                                     >
    {
      typedef iterator_facade<vertex_iterator,
                              vertex_descriptor const,
                              forward_traversal_tag> super_t;
    public:
      vertex_iterator(k_ary_tree const& g)
        : g(&g)
      {}

      vertex_iterator(vertex_descriptor start, k_ary_tree const& g)
        : last(g.null_vertex()), g(&g)
      {
        traversal.push(start);
        while (has_left_successor(traversal.top(), g))
          traversal.push(left_successor(traversal.top(), g));
      }

      typedef typename super_t::value_type value_type;
      typedef typename super_t::reference reference;

    private:
      friend class iterator_core_access;

      reference dereference() const
      {
        return traversal.top();
      }

      void increment()
      {
        if (has_right_successor(traversal.top(), *g))
        {
          if (right_successor(traversal.top(), *g) != last)
          {
            traversal.push(right_successor(traversal.top(), *g));
            while (has_left_successor(traversal.top(), *g))
              traversal.push(left_successor(traversal.top(), *g));
            return;
          }

        }

        do
        {
          last = traversal.top();
          traversal.pop();
        }
        while (!traversal.empty()
              && (!has_right_successor(traversal.top(), *g)
                  || right_successor(traversal.top(), *g) == last));
      }

      bool equal(vertex_iterator const &other) const
      {
        BOOST_ASSERT(g == other.g);

        if (traversal.empty())
          return other.traversal.empty();

        if (other.traversal.empty())
          return false;

        return traversal.top() == other.traversal.top();
      }

      vertex_descriptor last;
      std::stack<vertex_descriptor> traversal;
      k_ary_tree const *g;
    };

    friend
    std::pair<vertex_iterator, vertex_iterator>
    vertices(k_ary_tree const &g)
    {
      if (num_vertices(g) == 0)
        return std::make_pair(vertex_iterator(g), vertex_iterator(g));
      auto start = default_starting_vertex(g);
      return std::make_pair(vertex_iterator(start, g), vertex_iterator(g));
    }


    // *** MutableGraph interface ***

    friend
    std::pair<edge_descriptor, bool>
    add_edge(vertex_descriptor u, vertex_descriptor v, k_ary_tree &g)
    {
      return g.add_edge(u, v);
    }

    friend
    void
    remove_edge(vertex_descriptor u, vertex_descriptor v, k_ary_tree &g)
    {
      g.remove_edge(u, v);
    }

    friend
    void
    remove_edge(edge_descriptor e, k_ary_tree &g)
    {
      remove_edge(e.first, e.second, g);
    }

    friend
    void
    clear_vertex(vertex_descriptor u, k_ary_tree &g)
    {
      g.clear_vertex(u);
    }
  };


  /************************
   *  Bidirectional tree  *
   ************************/
  template <std::size_t K, typename Vertex>
  class k_ary_tree<K, true, Vertex>
    : public detail::k_ary_tree_base<K, Vertex,
          detail::k_ary_tree_bidirectional_node<k_ary_tree<K, true, Vertex> > >
  {
    typedef detail::k_ary_tree_base<K, Vertex,
    detail::k_ary_tree_bidirectional_node<k_ary_tree<K, true, Vertex> > > super_t;

  public:
    typedef bidirectional_tag directed_category;
    class traversal_category : public bidirectional_graph_tag,
                               public vertex_list_graph_tag {};
    typedef typename super_t::edge_descriptor edge_descriptor;
    typedef typename super_t::vertex_descriptor vertex_descriptor;
    typedef typename super_t::degree_size_type degree_size_type;
    using super_t::k;

    using super_t::super_t;

    friend
    void add_left_edge(vertex_descriptor parent, vertex_descriptor child,
                       k_ary_tree &g)
    {
      BOOST_ASSERT(parent != child);

      g.add_left_edge(parent, child);
      g.nodes[child].predecessor = parent;
    }

    friend
    void add_right_edge(vertex_descriptor parent, vertex_descriptor child,
                        k_ary_tree &g)
    {
      BOOST_ASSERT(parent != child);

      g.add_right_edge(parent, child);
      g.nodes[child].predecessor = parent;
    }


  private:
    struct make_in_edge_descriptor
    {
      make_in_edge_descriptor(vertex_descriptor target) : target(target) {}

      edge_descriptor operator()(vertex_descriptor source) const
      {
        return edge_descriptor(source, target);
      }

      vertex_descriptor target;
    };

  public:
    typedef transform_iterator<make_in_edge_descriptor, vertex_descriptor const *,
                                              edge_descriptor> in_edge_iterator;

    // *** VertexListGraph interface ***

    struct vertex_iterator : public iterator_facade<
                                                  vertex_iterator,
                                                  vertex_descriptor const,
                                                  multi_pass_input_iterator_tag
                                                     >
    {
      typedef iterator_facade<vertex_iterator,
                              vertex_descriptor const,
                              forward_traversal_tag> super_t;
    public:
      vertex_iterator(k_ary_tree const& g)
        : current(g.null_vertex()), last(g.null_vertex()), g(&g) {}

      vertex_iterator(vertex_descriptor start, k_ary_tree const& g)
        : current(start), last(g.null_vertex()), g(&g)
      {
        while (has_left_successor(current, g))
          current = (left_successor(current, g));
      }

      typedef typename super_t::value_type value_type;
      typedef typename super_t::reference reference;

    private:
      friend class iterator_core_access;

      reference dereference() const
      {
        return current;
      }

      void increment()
      {
        if (has_right_successor(current, *g))
        {
          if (right_successor(current, *g) != last)
          {
            current = (right_successor(current, *g));
            while (has_left_successor(current, *g))
              current = (left_successor(current, *g));
            return;
          }
        }

        do
        {
          last = current;
          current = predecessor(current, *g);
        }
        while (current != g->null_vertex()
              && (!has_right_successor(current, *g)
                  || right_successor(current, *g) == last));
      }

      bool equal(vertex_iterator const &other) const
      {
        return current == other.current
               && (last == other.last || current == g->null_vertex());
              // && *g == *other.g
              ;
      }

      vertex_descriptor current, last;
      k_ary_tree const *g;
    };

    friend
    std::pair<vertex_iterator, vertex_iterator>
    vertices(k_ary_tree const &g)
    {
      if (num_vertices(g) == 0)
        return std::make_pair(vertex_iterator(g), vertex_iterator(g));
      auto start = default_starting_vertex(g);
      return std::make_pair(vertex_iterator(start, g), vertex_iterator(g));
    }

    // NOTE: This function will be an infinite loop if called on a recurrent
    // tree (which is not a tree any more).
    friend
    vertex_descriptor
    root(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(!empty(u, g));

      while (has_predecessor(u, g))
      {
        u = predecessor(u, g);
      }

      BOOST_ASSERT(u != graph_traits<k_ary_tree>::null_vertex());
      return u;
    }

    friend
    bool
    is_left_successor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(!empty(u, g));

      vertex_descriptor v = predecessor(u, g);
      return left_successor(v, g) == u;
    }


    friend
    bool
    is_right_successor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(!empty(u, g));

      vertex_descriptor v = predecessor(u, g);
      return right_successor(v, g) == u;
    }

    friend
    bool
    has_predecessor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(!empty(u, g));
      BOOST_ASSERT(u < g.nodes.size());

      return g[u].predecessor != graph_traits<k_ary_tree>::null_vertex();
    }

    friend
    vertex_descriptor
    predecessor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(!empty(u, g));
      BOOST_ASSERT(u < g.nodes.size());

      return g[u].predecessor;
    }


    friend
    std::pair<in_edge_iterator, in_edge_iterator>
    in_edges(vertex_descriptor u, k_ary_tree const &g)
    {
      auto const i = g.nodes[u].predecessor != g.null_vertex();
      return std::make_pair(in_edge_iterator(&g.nodes[u].predecessor,
                                             make_in_edge_descriptor(u)),
                            in_edge_iterator(&g.nodes[u].predecessor + i,
                                             make_in_edge_descriptor(u)));
    }

    friend
    degree_size_type
    in_degree(vertex_descriptor v, k_ary_tree const &g)
    {
      return g.nodes[v].predecessor != g.null_vertex();
    }

    friend
    degree_size_type
    degree(vertex_descriptor v, k_ary_tree const &g)
    {
      return in_degree(v, g) + out_degree(v, g);
    }

    // *** MutableGraph interface ***

    friend
    std::pair<edge_descriptor, bool>
    add_edge(vertex_descriptor u, vertex_descriptor v, k_ary_tree &g)
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
    remove_edge(vertex_descriptor u, vertex_descriptor v, k_ary_tree &g)
    {
      BOOST_ASSERT(predecessor(v, g) == u);
      g.remove_edge(u, v);
      g.nodes[v].predecessor = super_t::null_vertex();
    }

    friend
    void
    remove_edge(edge_descriptor e, k_ary_tree &g)
    {
      remove_edge(e.first, e.second, g);
    }

    friend
    void
    clear_vertex(vertex_descriptor u, k_ary_tree &g)
    {
      g.clear_childrens_predecessor(u);
      g.clear_vertex(u);
      g.nodes[u].predecessor = super_t::null_vertex();
    }


    friend
    void
    remove_vertex(vertex_descriptor u, k_ary_tree &g)
    {
      BOOST_ASSERT(in_degree(u, g) == 0);

      g.remove_vertex(u);
    }


    void
    clear_childrens_predecessor(vertex_descriptor u)
    {
      for (int i = 0; i != k; i++)
      {
        super_t::nodes[super_t::nodes[u].successors[i]].predecessor = super_t::null_vertex();
      }
    }
  };

  // IncidenceGraph interface

  template <typename Vertex = std::size_t>
  using forward_binary_tree = k_ary_tree<2, false, Vertex>;
  template <typename Vertex = std::size_t>
  using bidirectional_binary_tree = k_ary_tree<2, true, Vertex>;

  namespace detail
  {
    template <typename BinaryTree, typename Visitor>
    Visitor
    traverse_nonempty(typename graph_traits<BinaryTree>::vertex_descriptor u,
                      BinaryTree const &g, Visitor vis)
    {
      BOOST_CONCEPT_ASSERT((concepts::ForwardBinaryTreeConcept<BinaryTree>));
      BOOST_ASSERT(!empty(u, g));

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
    int traverse_step(visit &v,
                      typename graph_traits<BinaryTree>::vertex_descriptor &u,
                      BinaryTree const &g)
    {
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree>));

      switch (v)
      {
        case visit::pre:
          if (has_left_successor(u, g)) {
                            u = left_successor(u, g);   return 1;
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
    Visitor traverse(typename graph_traits<BinaryTree>::vertex_descriptor u,
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
    bool
    bifurcate_isomorphic_nonempty(typename graph_traits<BinaryTree0>::vertex_descriptor u,
                                  BinaryTree0 const &g,
                                  typename graph_traits<BinaryTree1>::vertex_descriptor v,
                                  BinaryTree1 const &h)
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
        }
        else
          return false;
      }
      else
        if (has_left_successor(u, g))
          return false;

      if (has_right_successor(u, g)) {
        if (has_right_successor(v, h)) {
          if (!bifurcate_isomorphic_nonempty(right_successor(u, g), g,
                                             right_successor(v, h), h))
            return false;
        }
        else
          return false;
      }
      else
        if (has_right_successor(u, g))
          return false;

      return true;
    }


    template <typename BinaryTree0, typename BinaryTree1>
    bool
    bifurcate_isomorphic(typename graph_traits<BinaryTree0>::vertex_descriptor u,
                         BinaryTree0 const &g,
                         typename graph_traits<BinaryTree1>::vertex_descriptor v,
                         BinaryTree1 const &h)
    {
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree0>));
      BOOST_CONCEPT_ASSERT((concepts::BidirectionalBinaryTreeConcept<BinaryTree1>));

      if (empty(u, g))
        return empty(v, h);
      if (empty(v, h))
        return false;
      auto root0 = u;
      visit visit0 = visit::pre;
      visit visit1 = visit::pre;
      while (true) {
        traverse_step(visit0, u, g);
        traverse_step(visit1, v, h);
        if (visit0 != visit1)
          return false;
        if (u == root0 && visit0 == visit::post)
          return true;
      }
    }
  }


  template <typename Vertex, typename DFSTreeVisitor>
  void
  depth_first_search(k_ary_tree<2, false, Vertex> &g,
                    typename boost::graph_traits<
                              k_ary_tree<2, false, Vertex>
                                                >::vertex_descriptor s,
                    DFSTreeVisitor &vis)
  {
    if (!empty(s, g))
      vis = detail::traverse_nonempty(s, g, vis);
  }

  template <typename Vertex, typename DFSTreeVisitor>
  void
  depth_first_search(k_ary_tree<2, true, Vertex> &g,
                    typename boost::graph_traits<
                              k_ary_tree<2, false, Vertex>
                                                >::vertex_descriptor s,
                    DFSTreeVisitor &vis)
  {
    vis = detail::traverse(s, g, vis);
  }


  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(k_ary_tree<2, false, Vertex0> const &g,
              k_ary_tree<2, false, Vertex1> const &h)
  {
    if (num_vertices(g) != num_vertices(h))
      return false;
    return num_vertices(g) == 0
            || detail::bifurcate_isomorphic_nonempty(0, g, 0, h);
  }


  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(k_ary_tree<2, true, Vertex0> const &g,
              k_ary_tree<2, true, Vertex1> const &h)
  {
    if (num_vertices(g) != num_vertices(h))
      return false;
    return num_vertices(g) == 0 ||
            detail::bifurcate_isomorphic(0, g, 0, h);
  }
}

#endif // #if __cplusplus > 201103L

#endif // #ifndef BOOST_GRAPH_K_ARY_TREE
