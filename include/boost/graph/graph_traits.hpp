//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, University of Notre Dame, Notre
// Dame, IN 46556.
//
// Permission to modify the code and to distribute modified code is
// granted, provided the text of this NOTICE is retained, a notice that
// the code was modified is included with the above COPYRIGHT NOTICE and
// with the COPYRIGHT NOTICE in the LICENSE file, and that the LICENSE
// file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================

#ifndef BOOST_GRAPH_TRAITS_HPP
#define BOOST_GRAPH_TRAITS_HPP

#include <iterator>
#include <boost/utility.hpp>

namespace boost {
  
  template <typename G>
  struct graph_traits {
    typedef typename G::vertex_descriptor      vertex_descriptor;
    typedef typename G::edge_descriptor        edge_descriptor;
    typedef typename G::adjacency_iterator     adjacency_iterator;
    typedef typename G::out_edge_iterator      out_edge_iterator;
    typedef typename G::in_edge_iterator       in_edge_iterator;
    typedef typename G::vertex_iterator        vertex_iterator;
    typedef typename G::edge_iterator          edge_iterator;

    typedef typename G::directed_category      directed_category;
    typedef typename G::edge_parallel_category edge_parallel_category;

    typedef typename G::vertices_size_type     vertices_size_type;
    typedef typename G::edges_size_type        edges_size_type;
    typedef typename G::degree_size_type       degree_size_type;
  };

  // directed_category tags
  struct directed_tag { };
  struct undirected_tag { };
  struct bidirectional_tag : public directed_tag { };

  // edge_parallel_category tags
  struct allow_parallel_edge_tag {};
  struct disallow_parallel_edge_tag {};

  //?? not the right place ?? Lee
  struct multi_pass_input_iterator_tag : std::input_iterator_tag { };

} // namespace boost

/* Some helper functions for dealing with pairs as edges */
template <class T, class G>
T source(std::pair<T,T> p, const G&) { return p.first; }

template <class T, class G>
T target(std::pair<T,T> p, const G&) { return p.second; }

#endif /* BOOST_GRAPH_TRAITS_HPP*/
