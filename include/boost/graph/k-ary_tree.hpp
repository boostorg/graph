//=======================================================================
// Copyright 2018 Jeremy William Murphy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_K_ARY_TREE
#define BOOST_GRAPH_K_ARY_TREE

#include <boost/config.hpp>

#include <boost/graph/graph_traits.hpp>
#include <boost/graph/graph_mutability_traits.hpp>
#include <boost/graph/graph_selectors.hpp>
#include <boost/property_map/property_map.hpp>
#include <boost/graph/detail/indexed_properties.hpp>

#include <boost/iterator/transform_iterator.hpp>

#include <boost/array.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>

#include <boost/type_traits/conditional.hpp>

#include <algorithm>
#include <vector>

namespace boost
{
  namespace order {
    enum visit { pre, in, post };
  }

  template <typename Graph>
  bool
  empty(typename graph_traits<Graph>::vertex_descriptor u, Graph const &)
  {
    return u == graph_traits<Graph>::null_vertex();
  }

  namespace detail
  {
    template <typename Tree>
    struct k_ary_tree_forward_node
    {
      typedef typename graph_traits<Tree>::vertex_descriptor vertex_descriptor;

      k_ary_tree_forward_node()
      {
        fill(successors, graph_traits<Tree>::null_vertex());
      }

      array<vertex_descriptor, Tree::k> successors;
    };


    template <typename Tree>
    struct k_ary_tree_bidirectional_node : k_ary_tree_forward_node<Tree>
    {
      using typename k_ary_tree_forward_node<Tree>::vertex_descriptor;

      k_ary_tree_bidirectional_node(vertex_descriptor predecessor
                                    = graph_traits<Tree>::null_vertex())
        : predecessor(predecessor)
      {}

      vertex_descriptor predecessor;
    };

    template <std::size_t K, typename Vertex, typename Node>
    class k_ary_tree_base
    {
    public:
      BOOST_STATIC_CONSTEXPR std::size_t k = K;

      typedef Vertex vertex_descriptor;
      typedef vertex_descriptor const * vertex_iterator;

      BOOST_STATIC_CONSTEXPR vertex_descriptor null_vertex()
      {
        return vertex_descriptor(-1);
      }

      typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
      typedef disallow_parallel_edge_tag edge_parallel_category;
      typedef std::size_t degree_size_type;
      typedef std::size_t vertices_size_type;

    protected:

      struct make_out_edge_descriptor
      {
        make_out_edge_descriptor(vertex_descriptor source) : source(source) {}

        edge_descriptor operator()(vertex_descriptor target) const
        {
          return edge_descriptor(source, target);
        }

        vertex_descriptor source;
      };

    public:
      Node const&
      operator[](vertex_descriptor u) const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        return nodes[u];
      }

      template <std::size_t N>
      typename enable_if_c<N < K, bool>::type
      has_successor(vertex_descriptor u) const
      {
        return nodes[u].successors[N] != null_vertex();
      }


      typedef transform_iterator<make_out_edge_descriptor,
                                 vertex_descriptor const *,
                                 edge_descriptor> out_edge_iterator;

      k_ary_tree_base()
      {
        free_list.push_back(0);

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
      }

      // *** IncidenceGraph ***

      friend
      vertex_descriptor source(edge_descriptor e, k_ary_tree_base const &)
      {
        return e.first;
      }

      friend
      vertex_descriptor target(edge_descriptor e, k_ary_tree_base const &)
      {
        return e.second;
      }

      friend
      std::pair<out_edge_iterator, out_edge_iterator>
      out_edges(vertex_descriptor u, k_ary_tree_base const &g)
      {
        // TODO: Filter successors to skip null_vertex.
        return std::make_pair(out_edge_iterator(begin(g.nodes[u].successors),
                                                make_out_edge_descriptor(u)),
                              out_edge_iterator(end(g.nodes[u].successors),
                                                make_out_edge_descriptor(u)));
      }

      friend
      degree_size_type out_degree(vertex_descriptor v, k_ary_tree_base const &g)
      {
        return k - count(g.nodes[v].successors, null_vertex());
      }

      // *** VertexListGraph interface ***

      friend
      std::size_t num_vertices(k_ary_tree_base const &g)
      {
        return g.num_vertices();
      }

      // *** MutableGraph interface ***

      friend
      vertex_descriptor add_vertex(k_ary_tree_base &g)
      {
        return g.add_vertex();
      }

      friend
      void remove_vertex(vertex_descriptor u, k_ary_tree_base &g)
      {
        g.remove_vertex(u);
      }

      friend
      bool
      has_left_successor(Vertex u, k_ary_tree_base const &g)
      {
        return g.template has_successor<0>(u);
      }

      friend
      bool
      has_right_successor(Vertex u, k_ary_tree_base const &g)
      {
        return g.template has_successor<1>(u);
      }


      friend
      typename enable_if_c<K == 2, Vertex>::type
      left_successor(Vertex u, k_ary_tree_base const &g)
      {
        return g.nodes[u].successors[0];
      }

      friend
      typename enable_if_c<K == 2, Vertex>::type
      right_successor(Vertex u, k_ary_tree_base const &g)
      {
        return g.nodes[u].successors[1];
      }

    protected:
      // Adds an edge between vertices, adding them if necessary.
      std::pair<edge_descriptor, bool>
      add_edge(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());

        add_vertex(u);
        add_vertex(v);
        return add_edge_strict(u, v);
      }

      // Adds an edge between existing vertices.
      std::pair<edge_descriptor, bool>
      add_edge_strict(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);
        BOOST_ASSERT(u < nodes.size());
        BOOST_ASSERT(v < nodes.size());
        BOOST_ASSERT(find(free_list, u) == find(free_list, v));

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());

        array<vertex_descriptor, 2> const keys = {null_vertex(), v};
        vertex_descriptor *p = find_first_of(nodes[u].successors, keys); // O(k)
        edge_descriptor const result(u, v);

        if (p == end(nodes[u].successors) or *p == v)
          return std::make_pair(result, false);
        else
        {
          *p = v;
          return std::make_pair(result, true);
        }

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
      }


      void
      remove_edge(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);
        BOOST_ASSERT(u < nodes.size() && v < nodes.size());
        BOOST_ASSERT(find(free_list, u) == find(free_list, v)); // i.e., end.

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());

        vertex_descriptor * const p = find(nodes[u].successors, v);
        BOOST_ASSERT(p != end(nodes[u].successors));
        *p = null_vertex();
      }

      std::size_t num_vertices() const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        // Careful arithmetic so no underflow.
        return nodes.size() - (free_list.size() - 1);
      }

      vertex_descriptor add_vertex()
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());

        nodes.resize(std::max(vertex_descriptor(nodes.size()), free_list.back() + 1));
        vertex_descriptor const result = free_list.back();
        if (free_list.size() == 1)
          free_list.back() = nodes.size();
        else
          free_list.pop_back();
        BOOST_ASSERT(!free_list.empty());
        return result;
      }

      // Internal use.
      bool add_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());

        // First handle the case where it's not on the free list.
        if (nodes.size() < u) {
          for (std::size_t i = nodes.size(); i != u; i++)
            free_list.push_back(i);
          nodes.resize(u + 1);
          free_list[0] = nodes.size();
          BOOST_ASSERT(!free_list.empty());
          BOOST_ASSERT(free_list[0] == nodes.size());
          return true;
        }

        typedef typename std::vector<vertex_descriptor>::iterator iterator;
        iterator const which = find(free_list, u);
        if (which == end(free_list))
          return false;

        if (which == begin(free_list)) {
          (*which)++;
          nodes.resize(*which);
        }
        else
          free_list.erase(which);

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        return true;
      }


      void remove_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(num_vertices() > 0);
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(find(free_list, u) == end(free_list));
        // TODO: Assertions that u is disconnected.

        if (u == nodes.size() - 1)
        {
          // happens to be the last node
          nodes.pop_back();
          free_list[0]--;
        }
        else
        {
          free_list.push_back(u);
        }

        BOOST_ASSERT(find(free_list, u) != end(free_list));
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
      }

      std::vector<Node> nodes;
      std::vector<vertex_descriptor> free_list; // Keeps track of holes in storage.
    };
  }

  template <std::size_t K, bool Predecessor, typename Vertex = std::size_t>
  class k_ary_tree;

  // Directed (forward) tree
  template <std::size_t K, typename Vertex>
  class k_ary_tree<K, false, Vertex>
    : public detail::k_ary_tree_base<K, Vertex,
              detail::k_ary_tree_forward_node<k_ary_tree<K, false, Vertex> > >
  {
    typedef detail::k_ary_tree_base<K, Vertex,
      detail::k_ary_tree_forward_node<k_ary_tree<K, false, Vertex> > > super_t;

  public:
    typedef directed_tag directed_category;
    class traversal_category: public incidence_graph_tag {};
    using typename super_t::edge_descriptor;
    using typename super_t::vertex_descriptor;
    using typename super_t::vertex_iterator;

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

  };

  // Bidirectional tree
  template <std::size_t K, typename Vertex>
  class k_ary_tree<K, true, Vertex>
    : public detail::k_ary_tree_base<K, Vertex,
          detail::k_ary_tree_bidirectional_node<k_ary_tree<K, true, Vertex> > >
  {
    typedef detail::k_ary_tree_base<K, Vertex,
    detail::k_ary_tree_bidirectional_node<k_ary_tree<K, true, Vertex> > > super_t;

  public:
    typedef bidirectional_tag directed_category;
    class traversal_category : public bidirectional_graph_tag {};
    using typename super_t::vertex_descriptor;
    using typename super_t::vertex_iterator;
    using typename super_t::edge_descriptor;
    using typename super_t::degree_size_type;
    using super_t::k;

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

    friend
    bool
    is_left_successor(vertex_descriptor u, k_ary_tree const &g)
    {
      vertex_descriptor v = predecessor(u, g);
      return left_successor(v, g) == u;
    }


    friend
    bool
    is_right_successor(vertex_descriptor u, k_ary_tree const &g)
    {
      vertex_descriptor v = predecessor(u, g);
      return right_successor(v, g) == u;
    }

    friend
    bool
    has_predecessor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(u < g.nodes.size());
      return g[u].predecessor != graph_traits<k_ary_tree>::null_vertex();
    }

    friend
    vertex_descriptor
    predecessor(vertex_descriptor u, k_ary_tree const &g)
    {
      BOOST_ASSERT(u < g.nodes.size());
      return g[u].predecessor;
    }


    friend
    std::pair<in_edge_iterator, in_edge_iterator>
    in_edges(vertex_descriptor u, k_ary_tree const &g)
    {
      std::ptrdiff_t const i = g.nodes[u].predecessor != g.null_vertex();
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

  };

  // IncidenceGraph interface

  typedef k_ary_tree<2, false> forward_binary_tree;
  typedef k_ary_tree<2, true> bidirectional_binary_tree;

  namespace detail
  {
    template <typename Tree, typename Visitor>
    Visitor traverse_nonempty(typename graph_traits<Tree>::vertex_descriptor u,
                              Tree const &g, Visitor vis)
    {
      BOOST_ASSERT(u != graph_traits<Tree>::null_vertex());

      vis(order::pre, u);
      if (has_left_successor(u, g))
        vis = traverse_nonempty(left_successor(u, g), g, vis);
      vis(order::in, u);
      if (has_right_successor(u, g))
        vis = traverse_nonempty(right_successor(u, g), g, vis);
      vis(order::post, u);
      return vis;
    }

    template <typename Graph>
    int traverse_step(order::visit &v, typename graph_traits<Graph>::vertex_descriptor &u,
                      Graph const &g)
    {
      // Requires BidirectionalTree<Graph>
      switch (v)
      {
        case order::pre:
          if (has_left_successor(u, g)) {
            u = left_successor(u, g);
            return 1;
          }
          v = order::in;
          return 0;
        case order::in:
          if (has_right_successor(u, g)) {
            v = order::pre;
            u = right_successor(u, g);
            return 1;
          }
          v = order::post;
          return 0;
        case order::post:
          if (is_left_successor(u, g))
            v = order::in;
          u = predecessor(u, g);
          return -1;
      }
      // This is to silence the compiler warning about control reaches end of
      // non-void function, even though this code is unreachable.
      throw std::logic_error("Something magic and impossible happened.");
    }

    template <typename Graph, typename Visitor>
    Visitor traverse(typename graph_traits<Graph>::vertex_descriptor u,
                    Graph const &g, Visitor vis)
    {
      if (u == graph_traits<Graph>::null_vertex())
        return vis;
      typename graph_traits<Graph>::vertex_descriptor root = u;
      order::visit v = order::pre;
      vis(v, u);
      do {
        traverse_step(v, u, g);
        vis(v, u);
      } while (u != root || v != order::post);
      return vis;
    }


    template <typename Graph0, typename Graph1>
    bool
    bifurcate_isomorphic(typename graph_traits<Graph0>::vertex_descriptor u,
                          Graph0 const &g,
                         typename graph_traits<Graph1>::vertex_descriptor v,
                         Graph1 const &h)
    {
      if (empty(u, g))
        return empty(v, h);
      if (empty(v, h))
        return false;
      typename graph_traits<Graph0>::vertex_descriptor root0 = u;
      order::visit visit0 = order::pre;
      order::visit visit1 = order::pre;
      while (true) {
        traverse_step(visit0, u, g);
        traverse_step(visit1, v, h);
        if (visit0 != visit1)
          return false;
        if (u == root0 && visit0 == order::post)
          return true;
      }
    }
  }


  template <typename Vertex, typename DFSTreeVisitor, typename ColorMap>
  void
  depth_first_visit(k_ary_tree<2, false, Vertex> &g, Vertex s,
                    DFSTreeVisitor &vis, ColorMap)
  {
    vis = detail::traverse_nonempty(s, g, vis);
  }

  template <typename Vertex, typename DFSTreeVisitor, typename ColorMap>
  void
  depth_first_visit(k_ary_tree<2, true, Vertex> &g, Vertex s,
                    DFSTreeVisitor &vis, ColorMap)
  {
    vis = detail::traverse(s, g, vis);
  }

  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(k_ary_tree<2, false, Vertex0> const &g, k_ary_tree<2, false, Vertex1> const &h)
  {
    // return detail::bifurcate_isomorphic(u, g, v, h);
    return true;
  }


  template <typename Vertex0, typename Vertex1>
  bool
  isomorphism(k_ary_tree<2, true, Vertex0> const &g, k_ary_tree<2, true, Vertex1> const &h)
  {
    return detail::bifurcate_isomorphic(0, g, 0, h);
  }
}

#endif
