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
#include <boost/range/algorithm/find.hpp>
#include <boost/range/algorithm/fill.hpp>

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
    template <std::size_t K, bool Predecessor, typename Vertex>
    struct k_ary_tree_traits;

    // Directed (aka forward)
    template <std::size_t K, typename Vertex>
    struct k_ary_tree_traits<K, false, Vertex>
    {
      typedef directed_tag directed_category;
    };

    // Bidirectional
    template <std::size_t K, typename Vertex>
    struct k_ary_tree_traits<K, true, Vertex>
    {
      typedef bidirectional_tag directed_category;
    };
  }


  template <std::size_t K, bool Predecessor, typename Vertex = std::size_t>
  class k_ary_tree : public detail::k_ary_tree_traits<K, Predecessor, Vertex>
  {
    typedef detail::k_ary_tree_traits<K, Predecessor, Vertex> super_t;

  public:
    typedef Vertex vertex_descriptor;

    BOOST_STATIC_CONSTEXPR vertex_descriptor null_vertex()
    {
      return vertex_descriptor(-1);
    }

    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef disallow_parallel_edge_tag edge_parallel_category;
    class traversal_category: public incidence_graph_tag,
                              public adjacency_graph_tag,
                              public vertex_list_graph_tag {};
                              // TODO: edge_list_graph_tag
    typedef std::size_t degree_size_type;
    BOOST_STATIC_CONSTEXPR std::size_t k = K;

  private:
    struct k_ary_tree_forward_node
    {
      k_ary_tree_forward_node()
      {
        fill(successors, null_vertex());
      }

      template <typename VertexRange>
      k_ary_tree_forward_node(VertexRange const &successors)
        : successors(successors)
      {
      }

      array<vertex_descriptor, k> successors;
    };

    typedef typename array<vertex_descriptor, k>::iterator vertex_iterator;

    struct k_ary_tree_bidirectional_node : k_ary_tree_forward_node
    {
      k_ary_tree_bidirectional_node(vertex_descriptor predecessor = null_vertex())
        : predecessor(predecessor)
      {}

      vertex_descriptor predecessor;
    };

    struct make_edge_descriptor
    {
      make_edge_descriptor(vertex_descriptor u) : u(u) {}

      edge_descriptor operator()(vertex_descriptor v) const
      {
        return edge_descriptor(u, v);
      }

      vertex_descriptor u;
    };

    typedef typename conditional<Predecessor, k_ary_tree_bidirectional_node,
                                              k_ary_tree_forward_node>::type node;

  public:
    typedef transform_iterator<make_edge_descriptor, vertex_descriptor const *, edge_descriptor> out_edge_iterator;

    k_ary_tree()
    {
      free_list.push_back(0);
    }

    // *** IncidenceGraph ***

    friend
    vertex_descriptor source(edge_descriptor e, k_ary_tree const &)
    {
      return e.first;
    }

    friend
    vertex_descriptor target(edge_descriptor e, k_ary_tree const &)
    {
      return e.second;
    }

    friend
    std::pair<out_edge_iterator, out_edge_iterator>
    out_edges(vertex_descriptor u, k_ary_tree const &g)
    {
      return std::make_pair(out_edge_iterator(begin(g.nodes[u].successors), make_edge_descriptor(u)),
                            out_edge_iterator(end(g.nodes[u].successors), make_edge_descriptor(u)));
    }

    friend
    degree_size_type out_degree(vertex_descriptor, k_ary_tree const &)
    {
      return k;
    }

    // *** VertexListGraph interface ***

    friend
    std::size_t num_vertices(k_ary_tree const &g)
    {
      return g.num_vertices();
    }

    // *** MutableGraph interface ***

    friend
    vertex_descriptor add_vertex(k_ary_tree &g)
    {
      return g.add_vertex();
    }

    friend
    void remove_vertex(vertex_descriptor u, k_ary_tree &g)
    {
      g.remove_vertex(u);
    }

  private:
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

    std::vector<node> nodes;
    std::vector<vertex_descriptor> free_list; // Keeps track of holes in storage.
  };

  // IncidenceGraph interface

  typedef k_ary_tree<2, false> forward_binary_tree;
  typedef k_ary_tree<2, true> bidirectional_binary_tree;
}

#endif
