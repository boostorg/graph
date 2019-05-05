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
#include <stack>

namespace boost
{
  namespace detail
  {
    template <typename Tree>
    struct k_ary_tree_forward_node
    {
      typedef typename graph_traits<Tree>::vertex_descriptor vertex_descriptor;
      typedef array<vertex_descriptor, Tree::k> vertex_array_t;

      k_ary_tree_forward_node()
      {
        fill(successors, graph_traits<Tree>::null_vertex());
      }

      vertex_array_t successors;
    };


    template <typename Tree>
    struct k_ary_tree_bidirectional_node : k_ary_tree_forward_node<Tree>
    {
      typedef typename k_ary_tree_forward_node<Tree>::vertex_descriptor
      vertex_descriptor;

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

      BOOST_STATIC_CONSTEXPR vertex_descriptor null_vertex()
      {
        return vertex_descriptor(-1);
      }

      typedef std::pair<vertex_descriptor, vertex_descriptor> edge_descriptor;
      typedef disallow_parallel_edge_tag edge_parallel_category;
      typedef std::size_t degree_size_type;
      typedef std::size_t vertices_size_type;

    public:
      class out_edge_iterator
                : public boost::iterator_adaptor<out_edge_iterator,
                                                 vertex_descriptor const *,
                                                 edge_descriptor,
                                                 forward_traversal_tag,
                                                 edge_descriptor>
      {
      public:
        out_edge_iterator() {}

        out_edge_iterator(vertex_descriptor const *first,
                          vertex_descriptor const *last,
                          vertex_descriptor source)
          : out_edge_iterator::iterator_adaptor_(first), last(last),
            source(source)
        {
          BOOST_ASSERT(source != null_vertex());
          post_increment();
        }

      private:
        auto dereference() const
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

        vertex_descriptor const *last;
        vertex_descriptor source;
      };

      Node const&
      operator[](vertex_descriptor u) const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        BOOST_ASSERT(u < nodes.size());
        BOOST_ASSERT(find(free_list, u) == boost::end(free_list));

        return nodes[u];
      }

      template <std::size_t N>
      typename enable_if_c<N < K, bool>::type
      has_successor(vertex_descriptor u) const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        BOOST_ASSERT(u < nodes.size());
        BOOST_ASSERT(find(free_list, u) == boost::end(free_list));

        return nodes[u].successors[N] != null_vertex();
      }

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
        auto const &successors = g.nodes[u].successors;

        return std::make_pair(out_edge_iterator(boost::begin(successors),
                                                boost::end(successors), u),
                              out_edge_iterator(boost::end(successors),
                                                boost::end(successors), u));
      }

      friend
      degree_size_type
      out_degree(vertex_descriptor v, k_ary_tree_base const &g)
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

      friend
      Vertex default_starting_vertex(k_ary_tree_base const &g)
      {
        Vertex start = 0;
        if (g.free_list.size() != 1)
        {
          auto const not_successors = [](auto x, auto y){ return ++x != y; };
          auto q = adjacent_find(adaptors::reverse(g.free_list), not_successors);
          start = *q + 1;
        }
        return start;
      }
    protected:
      // Adds an edge between vertices, adding them if necessary.
      std::pair<edge_descriptor, bool>
      add_edge(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        add_vertex(u);
        add_vertex(v);
        auto const result = add_edge_strict(u, v);
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        return result;
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
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        array<vertex_descriptor, 2> const keys{{null_vertex(), v}};
        auto const p = find_first_of(nodes[u].successors, keys); // O(k)
        edge_descriptor const result(u, v);

        if (p == boost::end(nodes[u].successors) or *p == v)
          return std::make_pair(result, false);
        else
        {
          *p = v;
          return std::make_pair(result, true);
        }
      }


      void
      add_left_edge(vertex_descriptor parent, vertex_descriptor child)
      {
        BOOST_ASSERT(parent != child);
        BOOST_ASSERT(parent < nodes.size());
        BOOST_ASSERT(child < nodes.size());
        BOOST_ASSERT(find(free_list, parent) == find(free_list, child));

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        BOOST_ASSERT(!has_left_successor(parent, *this));

        nodes[parent].successors[0] = child;

        BOOST_ASSERT(has_left_successor(parent, *this));
        BOOST_ASSERT(left_successor(parent, *this) == child);
      }


      void
      add_right_edge(vertex_descriptor parent, vertex_descriptor child)
      {
        BOOST_ASSERT(parent != child);
        BOOST_ASSERT(parent < nodes.size());
        BOOST_ASSERT(child < nodes.size());
        BOOST_ASSERT(find(free_list, parent) == find(free_list, child));

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        BOOST_ASSERT(!has_right_successor(parent, *this));

        nodes[parent].successors[1] = child;

        BOOST_ASSERT(has_right_successor(parent, *this));
        BOOST_ASSERT(right_successor(parent, *this) == child);
      }


      void
      remove_edge(vertex_descriptor u, vertex_descriptor v)
      {
        BOOST_ASSERT(u != v);
        BOOST_ASSERT(u < nodes.size() && v < nodes.size());
        BOOST_ASSERT(find(free_list, u) == find(free_list, v)); // i.e., end.

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        auto const p = find(nodes[u].successors, v);
        BOOST_ASSERT(p != boost::end(nodes[u].successors));
        *p = null_vertex();
      }

      std::size_t num_vertices() const
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        return nodes.size() - free_list.size() + 1;
      }

      vertex_descriptor add_vertex()
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
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
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        return result;
      }

      // Internal use.
      bool add_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));

        // First handle the case where it's not on the free list.
        if (nodes.size() < u) {
          free_list.reserve(free_list.size() + u - nodes.size());
          for (auto i = u - 1; i != nodes.size() - 1; i--)
            free_list.push_back(i);
          nodes.resize(u + 1);
          free_list[0] = nodes.size();
          BOOST_ASSERT(!free_list.empty());
          BOOST_ASSERT(free_list[0] == nodes.size());
          return true;
        }

        auto const which = find(free_list, u);
        if (which == boost::begin(free_list)) {
          (*which)++;
          nodes.resize(*which);
        }
        else {
          if (which == boost::end(free_list))
            return false;
          free_list.erase(which);
        }

        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        return true;
      }


      void remove_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(u || nodes.size() == 1); // FIXME: Kludge for now.

        BOOST_ASSERT(num_vertices() > 0);
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
        BOOST_ASSERT(find(free_list, u) == boost::end(free_list));
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
            auto const not_successors = [](auto x, auto y){ return x != ++y; };
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
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(boost::is_sorted(free_list, std::greater<>()));
      }


      void clear_vertex(vertex_descriptor u)
      {
        BOOST_ASSERT(num_vertices() > 0);
        BOOST_ASSERT(!free_list.empty());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(free_list[0] == nodes.size());
        BOOST_ASSERT(find(free_list, u) == boost::end(free_list));

        fill(nodes[u].successors, null_vertex());
      }

      std::vector<Node> nodes;
      std::vector<vertex_descriptor> free_list; // Keeps track of holes.
    };
  } // namespace detail
} // namespace boost

#endif
