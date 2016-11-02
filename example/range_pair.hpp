//=======================================================================
// Copyright 2016 Murray Cumming
// Authors: Murray Cumming
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================

#ifndef BOOST_GRAPH_EDGE_LIST_HPP
#define BOOST_GRAPH_EDGE_LIST_HPP

// Inspired by:
// https://cplusplusmusings.wordpress.com/2013/04/14/range-based-for-loops-and-pairs-of-iterators/
template <typename T_Pair>
class range_pair
{
public:
  explicit range_pair(T_Pair pair)
  : pair_(pair)
  {}
  
  range_pair(const range_pair& src) = default;
  range_pair& operator=(const range_pair& src) = default;

  range_pair(range_pair&& src) = default;
  range_pair& operator=(range_pair&& src) = default;

  typename T_Pair::first_type
  begin() const
  { return pair_.first; }

  typename T_Pair::second_type
  end() const
  { return pair_.second; }

private:
  T_Pair pair_;
};

template <typename T_Pair>
range_pair<T_Pair> make_range_pair(T_Pair pair)
{
  return range_pair<T_Pair>(pair);
}

#endif //BOOST_GRAPH_EDGE_LIST_HPP

