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

#include <boost/array.hpp>
#include <boost/range.hpp>
#include <boost/range/algorithm/find.hpp>

#include <algorithm>
#include <vector>

namespace boost
{
  template <std::size_t K, typename Vertex>
  struct k_ary_tree_base_node
  {
    typedef Vertex vertex_type;
    BOOST_STATIC_CONSTEXPR std::size_t k = K;
  };

  template <std::size_t K, typename Vertex>
  struct k_ary_tree_forward_node : k_ary_tree_base_node<K, Vertex>
  {
    array<Vertex *, K> successors;
  };

  template <std::size_t K, typename Vertex>
  struct k_ary_tree_bidirectional_node : k_ary_tree_forward_node<K, Vertex>
  {
    Vertex *predecessor;
  };


  template <typename Vertex, typename BidirectionalTree>
  bool has_predecessor(Vertex u, BidirectionalTree const &t)
  {
    // Concept assert that t is BidirectionalTree?
    BOOST_ASSERT(u < num_vertices(t));
    return t[u].predecessor;
  }

  template <typename Vertex, typename BidirectionalTree>
  Vertex predecessor(Vertex u, BidirectionalTree const &t)
  {
    // Concept assert that t is BidirectionalTree?
    BOOST_ASSERT(u < num_vertices(t));
    BOOST_ASSERT(t[u].predecessor);
    return *t[u].predecessor;
  }

  namespace detail
  {
    template <std::size_t K, bool Predecessor, typename Vertex>
    struct k_ary_tree_traits;

    // Directed (aka forward)
    template <std::size_t K, typename Vertex>
    struct k_ary_tree_traits<K, false, Vertex>
    {
      typedef k_ary_tree_forward_node<K, Vertex> node;
      typedef directed_tag directed_category;
    };

    // Bidirectional
    template <std::size_t K, typename Vertex>
    struct k_ary_tree_traits<K, true, Vertex>
    {
      typedef k_ary_tree_bidirectional_node<K, Vertex> node;
      typedef bidirectional_tag directed_category;
    };
  }


  template <std::size_t K, bool Predecessor, typename Vertex = std::size_t>
  class k_ary_tree : public detail::k_ary_tree_traits<K, Predecessor, Vertex>
  {
    typedef detail::k_ary_tree_traits<K, Predecessor, Vertex> super_t;
    typedef typename detail::k_ary_tree_traits<K, Predecessor, Vertex>::node node;

  public:
    typedef Vertex vertex_descriptor;
    typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
    typedef disallow_parallel_edge_tag edge_parallel_category;
    class traversal_category: public incidence_graph_tag,
                              public adjacency_graph_tag,
                              public vertex_list_graph_tag {};
                              // TODO: edge_list_graph_tag
    typedef vertex_descriptor const * out_edge_iterator;
    typedef std::size_t degree_size_type;

    k_ary_tree()
    {
      free_list.push_back(0);
    }

    // *** VertexListGraph interface ***

    friend
    std::size_t num_vertices(k_ary_tree const &t)
    {
      return t.num_vertices();
    }

    // *** MutableGraph interface ***

    friend
    vertex_descriptor add_vertex(k_ary_tree &t)
    {
      return t.add_vertex();
    }

    friend
    void remove_vertex(vertex_descriptor u, k_ary_tree &t)
    {
      t.remove_vertex(u);
    }

  private:
    std::size_t num_vertices() const
    {
      BOOST_ASSERT(!free_list.empty());
      // Careful arithmetic so no over or underflows.
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

  typedef k_ary_tree<2, false> forward_binary_tree;
  typedef k_ary_tree<2, true> bidirectional_binary_tree;
}

#endif
