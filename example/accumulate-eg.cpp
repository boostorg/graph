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
#include <vector>
#include <list>
#include <algorithm>
#include <functional>
#include <cassert>

template < typename InputIterator, typename T, typename BinaryOperator >
  T accumulate(InputIterator first, InputIterator last, T init,
               BinaryOperator binary_op)
{
  for (; first != last; ++first)
    init = binary_op(init, *first);
  return init;
}


int
main()
{
  // using accumulate with a vector
  std::vector < double >x(10, 1.0);
  double sum1;
  sum1 = std::accumulate(x.begin(), x.end(), 0.0, std::plus < double >());

  // using accumulate with a linked list
  std::list < double >y;
  double sum2;
  // copy vector's values into the list
  std::copy(x.begin(), x.end(), std::back_inserter(y));
  sum2 = std::accumulate(y.begin(), y.end(), 0.0, std::plus < double >());
  assert(sum1 == sum2);         // they should be equal

  return 0;
}
