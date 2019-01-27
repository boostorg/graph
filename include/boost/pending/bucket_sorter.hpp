//
//=======================================================================
// Copyright 1997, 1998, 1999, 2000 University of Notre Dame.
// Authors: Andrew Lumsdaine, Lie-Quan Lee, Jeremy G. Siek
//
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
//=======================================================================
//
//
// Revision History:
//   13 June 2001: Changed some names for clarity. (Jeremy Siek)
//   01 April 2001: Modified to use new <boost/limits.hpp> header. (JMaddock)
//   28 Feb 2017: change bucket head, fix bug in remove. (Felix Salfelder)
//
#ifndef BOOST_GRAPH_DETAIL_BUCKET_SORTER_HPP
#define BOOST_GRAPH_DETAIL_BUCKET_SORTER_HPP

#include <vector>
#include <cassert>
#include <boost/limits.hpp>
#include <boost/concept/assert.hpp>
#include <boost/property_map/property_map.hpp>

namespace boost {

  template <class BucketType, class ValueType, class Bucket,
            class ValueIndexMap>
  class bucket_sorter {
    BOOST_CONCEPT_ASSERT(( ReadablePropertyMapConcept<ValueIndexMap, ValueType> ));
  public:
    typedef BucketType bucket_type;
    typedef ValueType value_type;
    typedef typename std::vector<value_type>::size_type size_type;

    bucket_sorter(size_type _length, bucket_type _max_bucket, 
                  const Bucket& _bucket = Bucket(), 
                  const ValueIndexMap& _id = ValueIndexMap()) 
      : next(_length+_max_bucket, invalid_value()),
        prev(_length, invalid_value()),
        head(next.size()?(next.begin()+_length):next.end()),
        id_to_value(_length),
        bucket(_bucket), id(_id) { }

    void remove(const value_type& x) {
      const size_type i = get(id, x);
      const size_type& next_node = next[i];
      const size_type& prev_node = prev[i];

      //check if i is the end of the bucket list
      if ( next_node != invalid_value() )
        prev[next_node] = prev_node;
      //update predecessor
      next[prev_node] = next_node;
    }

    void push(const value_type& x) {
      id_to_value[get(id, x)] = x;
      (*this)[bucket[x]].push(x);
    }

    void update(const value_type& x) {
      remove(x);
      (*this)[bucket[x]].push(x);
    }
    //  private:
    //    with KCC, the nested stack class is having access problems
    //    despite the friend decl.
    static size_type invalid_value() {
      return (std::numeric_limits<size_type>::max)();
    }

    typedef typename std::vector<size_type>::iterator Iter;
    typedef typename std::vector<value_type>::iterator IndexValueMap;

  public:
    friend class stack;

    class stack {
    public:
      stack(bucket_type _bucket_id, Iter h,
            Iter n, Iter p, IndexValueMap v, const ValueIndexMap& _id)
#if defined(BOOST_CLANG) && (1 == BOOST_CLANG) && defined(__APPLE_CC__)
      : bucket_id(_bucket_id), head(), next(), prev(), value(v), id(_id)
      {
        head = h;
        next = n;
        prev = p;
      }
#else
      : bucket_id(_bucket_id), head(h), next(n), prev(p), value(v), id(_id) {}
#endif

      // Avoid using default arg for ValueIndexMap so that the default
      // constructor of the ValueIndexMap is not required if not used.
      stack(bucket_type _bucket_id, Iter h,
            Iter n, Iter p, IndexValueMap v)
#if defined(BOOST_CLANG) && (1 == BOOST_CLANG) && defined(__APPLE_CC__)
        : bucket_id(_bucket_id), head(), next(), prev(), value(v)
      {
        head = h;
        next = n;
        prev = p;
      }
#else
        : bucket_id(_bucket_id), head(h), next(n), prev(p), value(v) {}
#endif

      void push(const value_type& x) {
        const size_type new_head = get(id, x);
        const size_type current = head[bucket_id];
        if ( current != invalid_value() )
          prev[current] = new_head;
        prev[new_head] = bucket_id + (head - next);
        next[new_head] = current;
        head[bucket_id] = new_head;
      }
      void pop() {
        size_type current = head[bucket_id];
        size_type next_node = next[current];
        head[bucket_id] = next_node;
        if ( next_node != invalid_value() )
          prev[next_node] = bucket_id + (head - next);
      }
      value_type& top() { return value[ head[bucket_id] ]; }
      const value_type& top() const { return value[ head[bucket_id] ]; }
      bool empty() const { return head[bucket_id] == invalid_value(); }
    private:
      bucket_type bucket_id;
      Iter head;
      Iter next;
      Iter prev;
      IndexValueMap value;
      ValueIndexMap id;
    };

    stack operator[](const bucket_type& i) {
      assert(i < next.size());
      return stack(i, head, next.begin(), prev.begin(),
                   id_to_value.begin(), id);
    }
  protected:
    std::vector<size_type>   next;
    std::vector<size_type>   prev;
    typename std::vector<size_type>::iterator head;
    std::vector<value_type>  id_to_value;
    Bucket bucket;
    ValueIndexMap id;
  };

}

#endif
