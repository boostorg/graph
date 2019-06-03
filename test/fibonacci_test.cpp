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

int main(void)
{
    using boost::heap::fibonacci_heap;

    typedef fibonacci_heap<int> heap_t;
    basic_interface<heap_t>();

    iterator_interface<fibonacci_heap<int> >();

    ordered_iterator_interface<fibonacci_heap<int> >();

    return EXIT_SUCCESS;
}
