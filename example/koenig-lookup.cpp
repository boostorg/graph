//=======================================================================
// Copyright 2001 Jeremy G. Siek, Andrew Lumsdaine, Lie-Quan Lee, 
//
// This file is part of the Boost Graph Library
//
// You should have received a copy of the License Agreement for the
// Boost Graph Library along with the software; see the file LICENSE.
// If not, contact Office of Research, Indiana University,
// Bloomington, IN 47405.
//
// Permission to modify the code and to distribute the code is
// granted, provided the text of this NOTICE is retained, a notice if
// the code was modified is included with the above COPYRIGHT NOTICE
// and with the COPYRIGHT NOTICE in the LICENSE file, and that the
// LICENSE file is distributed with the modified code.
//
// LICENSOR MAKES NO REPRESENTATIONS OR WARRANTIES, EXPRESS OR IMPLIED.
// By way of example, but not limitation, Licensor MAKES NO
// REPRESENTATIONS OR WARRANTIES OF MERCHANTABILITY OR FITNESS FOR ANY
// PARTICULAR PURPOSE OR THAT THE USE OF THE LICENSED SOFTWARE COMPONENTS
// OR DOCUMENTATION WILL NOT INFRINGE ANY PATENTS, COPYRIGHTS, TRADEMARKS
// OR OTHER RIGHTS.
//=======================================================================
#include <boost/graph/graph_traits.hpp>
namespace lib_jack
{
  class graph
  {                             /* ... */
  };
  int num_vertices(const graph &)
  {                             /* ... */
  }
}
namespace lib_jill
{
  class graph
  {                             /* ... */
  };
  int num_vertices(const graph &)
  {                             /* ... */
  }
}

namespace boost
{
  template <> struct graph_traits <lib_jack::graph >
  {
    typedef int vertices_size_type;
  };
    template <> struct graph_traits <lib_jill::graph >
  {
    typedef int vertices_size_type;
  };
}

namespace boost
{
  template < typename Graph > void pail(Graph & g)
  {
    typename graph_traits < Graph >::vertices_size_type N = num_vertices(g);    // Koenig lookup will resolve
    // ...
  }
}                               // namespace boost

int
main()
{
  lib_jack::graph g1;
  boost::pail(g1);
  lib_jill::graph g2;
  boost::pail(g2);
}
