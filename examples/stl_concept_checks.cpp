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
#include <boost/pending/concept_checks.hpp>
#include <boost/pending/concept_skeletons.hpp>

#include <iterator>
#include <set>
#include <map>
#include <vector>
#include <list>
#include <deque>
#ifndef BOOST_NO_SLIST
#include <slist>
#endif

int
main()
{
  typedef boost::trivial_iterator_skeleton<int> TrivIterSkeleton;
  typedef boost::mutable_trivial_iterator_skeleton<int> MutTrivIterSkeleton;
  REQUIRE(TrivIterSkeleton, TrivialIterator);
  REQUIRE(MutTrivIterSkeleton, Mutable_TrivialIterator);

#if defined(_ITERATOR_)
  // VC++ STL implementation is not standard conformant and
  // fails to pass these concept checks
#else
  typedef std::vector<int> Vector;
  typedef std::deque<int> Deque;
  typedef std::list<int> List;

  // VC++ missing pointer and const_pointer typedefs
  REQUIRE(Vector, Mutable_RandomAccessContainer);
  REQUIRE(Vector, BackInsertionSequence);

#if !defined(__GNUC__)
#if !defined __sgi
  // old deque iterator missing n + iter operation
  REQUIRE(Deque, Mutable_RandomAccessContainer);
#endif
  // warnings about signed and unsigned in old deque version
  REQUIRE(Deque, FrontInsertionSequence);
  REQUIRE(Deque, BackInsertionSequence);
#endif

  // VC++ missing pointer and const_pointer typedefs
  REQUIRE(List, Mutable_ReversibleContainer);
  REQUIRE(List, FrontInsertionSequence);
  REQUIRE(List, BackInsertionSequence);

#ifndef BOOST_NO_SLIST
  typedef std::slist<int> SList;
  REQUIRE(SList, FrontInsertionSequence);
#endif

  typedef std::set<int> Set;
  typedef std::multiset<int> MultiSet;
  typedef std::map<int,int> Map;
  typedef std::multimap<int,int> MultiMap;

  REQUIRE(Set, SortedAssociativeContainer);
  REQUIRE(Set, SimpleAssociativeContainer);
  REQUIRE(Set, UniqueAssociativeContainer);

  REQUIRE(MultiSet, SortedAssociativeContainer);
  REQUIRE(MultiSet, SimpleAssociativeContainer);
  REQUIRE(MultiSet, MultipleAssociativeContainer);

  REQUIRE(Map, SortedAssociativeContainer);
  REQUIRE(Map, UniqueAssociativeContainer);
  REQUIRE(Map, PairAssociativeContainer);

  REQUIRE(MultiMap, SortedAssociativeContainer);
  REQUIRE(MultiMap, MultipleAssociativeContainer);
  REQUIRE(MultiMap, PairAssociativeContainer);
#endif

  return 0;
}
