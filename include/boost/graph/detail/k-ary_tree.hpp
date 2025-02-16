//=======================================================================
// Copyright 2018 Jeremy William Murphy
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_DETAIL_K_ARY_TREE
#define BOOST_GRAPH_DETAIL_K_ARY_TREE

#include <boost/config.hpp>

#include <boost/array.hpp>

#include <boost/graph/graph_traits.hpp>

#include <boost/iterator/filter_iterator.hpp>
#include <boost/iterator/iterator_adaptor.hpp>

#include <boost/range.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/algorithm_ext/is_sorted.hpp>

#include <algorithm>
#include <cstddef>
#include <utility>
#include <vector>

namespace boost
{
  template <typename Graph>
  using vertex_descriptor_t = typename graph_traits<Graph>::vertex_descriptor;

  namespace detail
  {

    template <typename BinaryTree>
    struct binary_tree_forward_node
    {
      using vertex_descriptor = vertex_descriptor_t<BinaryTree>;

      binary_tree_forward_node()
      {
        boost::fill(successors, graph_traits<BinaryTree>::null_vertex());
      }

      array<vertex_descriptor, 2> successors;
    };

    template <typename BinaryTree>
    struct binary_tree_bidirectional_node
      : binary_tree_forward_node<BinaryTree>
    {
      using vertex_descriptor = vertex_descriptor_t<BinaryTree>;

      binary_tree_bidirectional_node(
        vertex_descriptor predecessor = graph_traits<BinaryTree>::null_vertex())
        : predecessor(predecessor)
      {}

      vertex_descriptor predecessor;
    };

    template <typename Vertex, typename Node>
    class binary_tree_base
    {
    public:
      typedef Vertex vertex_descriptor;
      typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
      typedef disallow_parallel_edge_tag edge_parallel_category;
      typedef std::size_t degree_size_type;
      typedef std::size_t vertices_size_type;

    private:
      void invariants() const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
      }

      void strict_invariants(vertex_descriptor u) const
      {
        invariants();
        BOOST_ASSERT(u < nodes.size());
        BOOST_ASSERT(boost::find(free_list, u) == boost::end(free_list));
      }

      void strict_invariants(vertex_descriptor u, vertex_descriptor v) const
      {
        invariants();
        BOOST_ASSERT(u != v);
        BOOST_ASSERT(u < nodes.size());
        BOOST_ASSERT(v < nodes.size());
        BOOST_ASSERT(find(free_list, u) == find(free_list, v));
      }


    protected:
      std::vector<Node> nodes;
      std::vector<Vertex> free_list;

    public:
      BOOST_STATIC_CONSTEXPR
      vertex_descriptor null_vertex()
      {
        return vertex_descriptor(-1);
      }

      binary_tree_base(Vertex n) : nodes(n), free_list{{n}} {}

      std::size_t num_vertices() const
      {
        invariants();
        return nodes.size() - free_list.size() + 1;
      }

      Node const &
      operator[](vertex_descriptor u) const
      {
        strict_invariants(u);
        return nodes[u];
      }

      template <std::size_t N>
      bool has_successor(vertex_descriptor u) const
      {
        strict_invariants(u);
        return nodes[u].successors[N] != null_vertex();
      }

      binary_tree_base()
      {
        free_list.push_back(0);
        invariants();
      }

      // *** IncidenceGraph ***

      class out_edge_iterator
        : public boost::iterator_adaptor<out_edge_iterator,
                                         vertex_descriptor const *,
                                         edge_descriptor,
                                         forward_traversal_tag,
                                         edge_descriptor>
      {
        vertex_descriptor const *last;
        vertex_descriptor source;

      public:
        out_edge_iterator(Vertex const *first, Vertex const *last, Vertex source)
          : out_edge_iterator::iterator_adaptor_(first), last(last),
            source(source)
        {
          BOOST_ASSERT(source != null_vertex());
          post_increment();
        }

      private:
        edge_descriptor dereference() const
        {
          return edge_descriptor(source, *this->base_reference());
        }

        void post_increment()
        {
          while (this->base_reference() != last
                 && *this->base_reference() == null_vertex())
          {
            this->base_reference()++;
          }
        }

        void increment()
        {
          this->base_reference()++;
          post_increment();
        }

        friend class boost::iterator_core_access;
      };

      friend
      vertex_descriptor source(edge_descriptor e, binary_tree_base const &)
      {
        return e.first;
      }

      friend
      vertex_descriptor target(edge_descriptor e, binary_tree_base const &)
      {
        return e.second;
      }

      friend
      std::pair<out_edge_iterator, out_edge_iterator>
      out_edges(vertex_descriptor u, binary_tree_base const &g)
      {
        auto const &successors = g.nodes[u].successors;

        return std::make_pair(out_edge_iterator(boost::begin(successors),
                                                boost::end(successors), u),
                              out_edge_iterator(boost::end(successors),
                                                boost::end(successors), u));
      }

      friend
      degree_size_type
      out_degree(vertex_descriptor v, binary_tree_base const &g)
      {
        return 2 - count(g.nodes[v].successors, null_vertex());
      }

      // *** VertexListGraph interface ***

      struct vertex_iterator
        : public iterator_facade <vertex_iterator, vertex_descriptor,
          multi_pass_input_iterator_tag, vertex_descriptor const &>
      {
        typedef iterator_facade<vertex_iterator, vertex_descriptor,
                multi_pass_input_iterator_tag, vertex_descriptor const &> super_t;
        typedef typename super_t::value_type value_type;
        typedef typename super_t::reference reference;

        vertex_descriptor last;
        std::stack<vertex_descriptor> traversal;
        binary_tree_base const *g;
      public:
        vertex_iterator(binary_tree_base const &g) : g(&g) {}

        vertex_iterator(vertex_descriptor start, binary_tree_base const &g)
          : last(g.null_vertex()), g(&g)
        {
          traversal.push(start);
          while (has_left_successor(traversal.top(), g))
            traversal.push(left_successor(traversal.top(), g));
        }

      private:
        friend class boost::iterator_core_access;

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
      };

      friend
      std::size_t num_vertices(binary_tree_base const &g)
      {
        return g.num_vertices();
      }

      friend
      std::pair<vertex_iterator, vertex_iterator>
      vertices(binary_tree_base const &g)
      {
        if (g.num_vertices() == 0)
          return std::make_pair(vertex_iterator(g), vertex_iterator(g));
        auto const start = default_starting_vertex(g);
        return std::make_pair(vertex_iterator(start, g), vertex_iterator(g));
      }

      // *** MutableGraph interface ***

      friend
      vertex_descriptor add_vertex(binary_tree_base &g)
      {
        return g.add_vertex();
      }

      friend
      void remove_vertex(vertex_descriptor u, binary_tree_base &g)
      {
        g.remove_vertex(u);
      }

      friend
      bool
      has_left_successor(Vertex u, binary_tree_base const &g)
      {
        return g.template has_successor<0>(u);
      }

      friend
      bool
      has_right_successor(Vertex u, binary_tree_base const &g)
      {
        return g.template has_successor<1>(u);
      }


      friend
      Vertex
      left_successor(Vertex u, binary_tree_base const &g)
      {
        return g.nodes[u].successors[0];
      }

      friend
      Vertex
      right_successor(Vertex u, binary_tree_base const &g)
      {
        return g.nodes[u].successors[1];
      }

      void clear()
      {
        nodes.clear();
        free_list.resize(1);
        free_list[0] = 0;
        invariants();
      }

      void shrink_to_fit()
      {
        nodes.shrink_to_fit();
        free_list.shrink_to_fit();
      }

      friend
      Vertex default_starting_vertex(binary_tree_base const &g)
      {
        Vertex start = 0;
        if (g.free_list.size() != 1)
        {
          auto const not_successors = [](auto x, auto y) { return ++x != y; };
          auto const q = boost::adjacent_find(adaptors::reverse(g.free_list),
                                              not_successors);
          start = *q + 1;
        }
        return start;
      }

    protected:
      /**************************
       * MutableGraph interface *
       **************************/

      // Adds an edge between vertices, adding them if necessary.
      std::pair<edge_descriptor, bool>
      add_edge(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);
        invariants();
        add_vertex(u);
        add_vertex(v);
        auto const result = add_edge_strict(u, v);
        invariants();
        return result;
      }

      // Adds an edge between existing vertices.
      std::pair<edge_descriptor, bool>
      add_edge_strict(vertex_descriptor u, vertex_descriptor v)
      {
        strict_invariants(u, v);
        array<vertex_descriptor, 2> const keys{{null_vertex(), v}};
        auto const p = find_first_of(nodes[u].successors, keys); // O(k)
        edge_descriptor const result(u, v);

        if (p == boost::end(nodes[u].successors) || *p == v)
          return std::make_pair(result, false);
        else
        {
          *p = v;
          return std::make_pair(result, true);
        }
      }


      void
      remove_edge(vertex_descriptor u, vertex_descriptor v)
      {
        strict_invariants(u, v);
        auto const p = find(nodes[u].successors, v);
        BOOST_ASSERT(p != boost::end(nodes[u].successors));
        *p = null_vertex();
      }

      vertex_descriptor add_vertex()
      {
        invariants();
        BOOST_ASSERT(nodes.size() >= free_list.back() + 1
                     || free_list.size() == 1);

        vertex_descriptor const result = free_list.back();
        if (free_list.size() == 1)
        {
          nodes.resize(result + 1);
          free_list.back() = nodes.size();
        }
        else
          free_list.pop_back();
        invariants();
        return result;
      }

      // Internal use.
      bool add_vertex(vertex_descriptor u)
      {
        invariants();
        // First handle the case where it's not on the free list.
        if (nodes.size() < u)
        {
          free_list.reserve(free_list.size() + u - nodes.size());
          for (auto i = u - 1; i != nodes.size() - 1; i--)
            free_list.push_back(i);
          nodes.resize(u + 1);
          free_list[0] = nodes.size();
          invariants();
          return true;
        }

        auto const which = find(free_list, u);
        if (which == boost::begin(free_list))
        {
          (*which)++;
          nodes.resize(*which);
        }
        else
        {
          if (which == boost::end(free_list))
            return false;
          free_list.erase(which);
        }

        invariants();
        return true;
      }


      void remove_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(u || nodes.size() == 1); // FIXME: Kludge for now.

        BOOST_ASSERT(num_vertices() > 0);
        strict_invariants(u);
        BOOST_ASSERT(out_degree(u, *this) == 0);

        if (u == nodes.size() - 1)
        {
          // happens to be the last node
          if (free_list.size() == 1)
          {
            // simple case, no other nodes to consider
            free_list[0]--;
            nodes.pop_back();
          }
          else
          {
            // might be a run of nodes to erase
            auto const not_successors = [](auto x, auto y) { return x != ++y; };
            auto const least = boost::adjacent_find(free_list, not_successors);
            auto const n = least - boost::begin(free_list);
            nodes.erase(boost::end(nodes) - n, boost::end(nodes)); //pop_back(n)
            free_list.erase(boost::begin(free_list), least); //pop_front(n) :/
          }
        }
        else
        {
          auto const here = boost::lower_bound(free_list, u, std::greater<>());
          free_list.insert(here, u);
        }

        BOOST_ASSERT(find(free_list, u) != boost::end(free_list));
        invariants();
      }


      void clear_vertex(vertex_descriptor u)
      {
        invariants();
        BOOST_ASSERT(num_vertices() > 0);

        boost::fill(nodes[u].successors, null_vertex());
      }

      /*******************************
       * MutableBinaryTree interface *
       *******************************/

      edge_descriptor
      add_left_edge(vertex_descriptor parent, vertex_descriptor child)
      {
        strict_invariants(parent, child);
        BOOST_ASSERT(!has_left_successor(parent, *this));

        nodes[parent].successors[0] = child;

        BOOST_ASSERT(has_left_successor(parent, *this));
        BOOST_ASSERT(left_successor(parent, *this) == child);

        return {parent, child};
      }


      edge_descriptor
      add_right_edge(vertex_descriptor parent, vertex_descriptor child)
      {
        strict_invariants(parent, child);
        BOOST_ASSERT(!has_right_successor(parent, *this));

        nodes[parent].successors[1] = child;

        BOOST_ASSERT(has_right_successor(parent, *this));
        BOOST_ASSERT(right_successor(parent, *this) == child);

        return {parent, child};
      }
    };
  } // namespace detail
} // namespace boost

#endif
