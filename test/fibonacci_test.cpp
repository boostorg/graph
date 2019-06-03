/*=============================================================================
    Copyright (c) 2010 Tim Blechmann

    Use, modification and distribution is subject to the Boost Software
    License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
    http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#include <iostream>
#include <iomanip>

#include "../../../boost/heap/fibonacci_heap.hpp"

using std::cout;
using std::endl;
using namespace boost::heap;

template <template<class...> class PriorityQueue>
void simple(void)
{
  PriorityQueue<int> pq;
}

//[ basic_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void basic_interface(void)
{
    PriorityQueue pq;

    pq.push(2);
    pq.push(3);
    pq.push(1);

    pq.pop();
    pq.pop();
    pq.pop();
}
//]

//[ iterator_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void iterator_interface(void)
{
    PriorityQueue pq;

    pq.push(2);
    pq.push(3);
    pq.push(1);

}
//]

//[ ordered_iterator_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void ordered_iterator_interface(void)
{
    PriorityQueue pq;

    pq.push(2);
    pq.push(3);
    pq.push(1);

    typename PriorityQueue::ordered_iterator begin = pq.ordered_begin();
    typename PriorityQueue::ordered_iterator end = pq.ordered_end();

}
//]


//[ merge_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void merge_interface(void)
{
    PriorityQueue pq;

    pq.push(3);
    pq.push(5);
    pq.push(1);

    PriorityQueue pq2;

    pq2.push(2);
    pq2.push(4);
    pq2.push(0);

    pq.merge(pq2);

    while (!pq.empty()) {
        pq.pop();
    }

    while (!pq2.empty()) {
        pq2.pop();
    }
}
//]

//[ heap_merge_algorithm
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void heap_merge_algorithm(void)
{
    PriorityQueue pq;

    pq.push(3);
    pq.push(5);
    pq.push(1);

    PriorityQueue pq2;

    pq2.push(2);
    pq2.push(4);
    pq2.push(0);

    boost::heap::heap_merge(pq, pq2);

    while (!pq.empty()) {
        pq.pop();
    }

    while (!pq2.empty()) {
        pq2.pop();
    }
}
//]

//[ mutable_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void mutable_interface(void)
{
    PriorityQueue pq;
    typedef typename PriorityQueue::handle_type handle_t;

    handle_t t3 = pq.push(3);
    handle_t t5 = pq.push(5);
    handle_t t1 = pq.push(1);

    pq.update(t3, 4);
    pq.increase(t5, 7);
    pq.decrease(t1, 0);

    while (!pq.empty()) {
        pq.pop();
    }
}
//]

//[ mutable_fixup_interface
// PriorityQueue is expected to be a max-heap of integer values
template <typename PriorityQueue>
void mutable_fixup_interface(void)
{
    PriorityQueue pq;
    typedef typename PriorityQueue::handle_type handle_t;

    handle_t t3 = pq.push(3);
    handle_t t5 = pq.push(5);
    handle_t t1 = pq.push(1);

    *t3 = 4;
    pq.update(t3);

    *t5 = 7;
    pq.increase(t5);

    *t1 = 0;
    pq.decrease(t1);

    while (!pq.empty()) {
        pq.pop();
    }
}
//]

//[ mutable_interface_handle_in_value
struct heap_data
{
    fibonacci_heap<heap_data>::handle_type handle;
    int payload;

    heap_data(){}

    heap_data(int i):
        payload(i)
    {}

    bool operator<(heap_data const & rhs) const
    {
        return payload < rhs.payload;
    }
};

template <template<class...> class PriorityQueue>
void mutable_interface_handle_in_value(void)
{

    typedef PriorityQueue<heap_data> heap_t;

    BOOST_CONCEPT_ASSERT(( MergablePriorityQueue<heap_t> ));

    heap_t heap;
    heap_data f(2);

    typename heap_t::handle_type handle = heap.push(f);
    (*handle).handle = handle; // store handle in node
}
//]


int main(void)
{
    using boost::heap::fibonacci_heap;

    simple<fibonacci_heap>();

    typedef fibonacci_heap<int> heap_t;
    basic_interface<heap_t>();

    iterator_interface<fibonacci_heap<int> >();

    ordered_iterator_interface<fibonacci_heap<int> >();

    merge_interface<fibonacci_heap<int> >();

    mutable_interface<fibonacci_heap<int> >();

    mutable_fixup_interface<fibonacci_heap<int> >();

    mutable_interface_handle_in_value<fibonacci_heap>();

    return EXIT_SUCCESS;
}
