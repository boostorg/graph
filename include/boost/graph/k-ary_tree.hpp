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


  template <typename Vertex, typename BidirectionalTree>
  bool has_predecessor(Vertex u, BidirectionalTree const &g)
  {
    // Concept assert that t is BidirectionalTree?
    BOOST_ASSERT(u < num_vertices(g));
    return g[u].predecessor;
  }

  template <typename Vertex, typename BidirectionalTree>
  Vertex predecessor(Vertex u, BidirectionalTree const &g)
  {
    // Concept assert that t is BidirectionalTree?
    BOOST_ASSERT(u < num_vertices(g));
    BOOST_ASSERT(g[u].predecessor);
    return *g[u].predecessor;
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

      k_ary_tree_bidirectional_node(vertex_descriptor predecessor = graph_traits<Tree>::null_vertex())
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
      typedef transform_iterator<make_out_edge_descriptor,
                                 vertex_descriptor const *,
                                 edge_descriptor> out_edge_iterator;

      k_ary_tree_base()
      {
        free_list.push_back(0);
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

    protected:
      std::size_t num_vertices() const
      {
        BOOST_ASSERT(!free_list.empty());
        // Careful arithmetic so no underflow.
        return nodes.size() - (free_list.size() - 1);
      }

      vertex_descriptor add_vertex()
      {
        BOOST_ASSERT(!free_list.empty());

        nodes.resize(std::max(vertex_descriptor(nodes.size()), free_list.back() + 1));
        vertex_descriptor const result = free_list.back();
        if (free_list.size() == 1)
          free_list.back() = nodes.size();
        else
          free_list.pop_back();
        return result;
      }

      void remove_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(num_vertices() > 0);
        BOOST_ASSERT(!free_list.empty());
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
      }

      std::vector<Node> nodes;
      std::vector<vertex_descriptor> free_list; // Keeps track of holes in storage.
    };
  }

  template <std::size_t K, bool Predecessor, typename Vertex = std::size_t>
  class k_ary_tree;

  template <std::size_t K, typename Vertex>
  class k_ary_tree<K, false, Vertex>
    : public detail::k_ary_tree_base<K, Vertex,
              detail::k_ary_tree_forward_node<k_ary_tree<K, false, Vertex> > >
  {
  public:
    typedef directed_tag directed_category;
    class traversal_category: public incidence_graph_tag {};
    using typename super_t::edge_descriptor;
    using typename super_t::vertex_descriptor;
    using typename super_t::vertex_iterator;

  };

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
    std::pair<in_edge_iterator, in_edge_iterator>
    in_edges(vertex_descriptor u, k_ary_tree const &g)
    {
      return std::make_pair(in_edge_iterator(&g.nodes[u].predecessor,
                                             make_in_edge_descriptor(u)),
                            in_edge_iterator(&g.nodes[u].predecessor + 1,
                                             make_in_edge_descriptor(u)));
    }

    friend
    degree_size_type in_degree(vertex_descriptor v, k_ary_tree const &g)
    {
      return g.nodes[v].predecessor != g.null_vertex();
    }

    friend
    degree_size_type degree(vertex_descriptor v, k_ary_tree const &g)
    {
      return in_degree(v, g) + out_degree(v, g);
    }

  };

  // IncidenceGraph interface

  typedef k_ary_tree<2, false> forward_binary_tree;
  typedef k_ary_tree<2, true> bidirectional_binary_tree;
}

#endif
